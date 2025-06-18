#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QColor>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <QTimer>
#include <QMenu>
#include <QDesktopServices>
#include <QTime>

#include "model/thought.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/markdown_block_widget.h"
#include "widgets/markdown_edit_widget.h"
#include "model/new_text_model.h"

MarkdownEditWidget::MarkdownEditWidget(QWidget *widget, Style *style)
	: BaseWidget(widget, style),
	m_cursor(nullptr, -1, 0)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
	setStyleSheet(
		QString("background-color: %1;")
			.arg(style->background().name(QColor::HexRgb))
	);

	m_layout = new QVBoxLayout(nullptr);
	m_layout->setSpacing(16);
	m_layout->setContentsMargins(QMargins(0, 0, 0, 0));
	setLayout(m_layout);
}

MarkdownEditWidget::~MarkdownEditWidget() {
	if (m_saveTimer != nullptr) {
		delete m_saveTimer;
		m_saveTimer = nullptr;
	}
	for (auto block: m_blocks)
		(*block).deleteLater();
	delete m_layout;
}

void MarkdownEditWidget::load(QString data) {
	QRegularExpression splitExp(
		"(\\n|\r\\n)", QRegularExpression::MultilineOption
	);
	QStringList lines = data.split(splitExp);
	m_model = text::TextModel(lines);

	// Clear old blocks.
	QLayoutItem *child;
	while (m_layout->count() != 0) {
		child = m_layout->takeAt(0);
		if (child->widget() != nullptr) {
			delete child->widget();
		}
		delete child;
	}
	m_blocks.clear();

	// Create blocks.
	QList<text::Paragraph> *list = m_model.paragraphs();

	if (list->size() == 0) {
		QString empty = "";
		text::Line line = text::Line(empty, true);
		text::Paragraph emptyParagraph = text::Paragraph(text::Text, line);
		list->push_back(emptyParagraph);
	}

	for (qsizetype i = 0; i < list->size(); ++i) {
		text::Paragraph *p = &((*list)[i]);
		QList<text::Line> *lines = p->getLines();

		MarkdownBlock *block = new MarkdownBlock(nullptr, m_style, this);
		connect(
			this, &MarkdownEditWidget::onCursorMove,
			block, &MarkdownBlock::onCursorMove
		);

		if (i == 0)
			block->setPlaceholder(tr("Start typing here..."));
		block->setParagraph(p);

		m_blocks.push_back(block);
		m_layout->addWidget(block);
	}

	m_layout->addStretch();
	updateGeometry();
}

void MarkdownEditWidget::setPresenter(MarkdownEditPresenter *p) {
	m_presenter = p;
}

bool MarkdownEditWidget::isDirty() const {
	return m_isDirty;
}

QString MarkdownEditWidget::text() {
	return m_model.text();
}

Style *MarkdownEditWidget::style() {
	return m_style;
}

// Events.

void MarkdownEditWidget::resizeEvent(QResizeEvent *event) {
	BaseWidget::resizeEvent(event);
}

void MarkdownEditWidget::mousePressEvent(QMouseEvent *event) {
	bool found = false;
	MarkdownCursor cursor = cursorAtPoint(event->pos(), &found);

	if (found) {
		if (event->button() == Qt::RightButton) {
			// Show menu.
			showContextMenu(event);
			return;
		} else if (event->modifiers() & Qt::ShiftModifier) {
			// When shift-click, activate selection from previous to current
			// position.
			m_selection.active = true;
			m_selection.end = cursor;
		} else {
			// Detect link.
			m_pressPoint = event->pos();
			checkForLinksUnderCursor(cursor);
			// Set initial cursor position.
			MarkdownCursor prev = m_cursor;
			processCursorMove(prev, cursor);
			// Adjust selection.
			m_selection.active = false;
			m_selection.start = m_cursor;
			m_selection.end = m_cursor;
		}

		update();

		if (!hasFocus())
			setFocus(Qt::MouseFocusReason);
	} else if (found = cursorAbovePoint(event->pos(), &cursor); found) {
		// Set initial cursor position.
		MarkdownCursor prev = m_cursor;
		processCursorMove(prev, cursor);
		// Adjust selection.
		m_selection.active = false;
		m_selection.start = m_cursor;
		m_selection.end = m_cursor;

		if (!hasFocus()) {
			setFocus();
		}
	} else {
		clearFocus();
	}
}

void MarkdownEditWidget::mouseMoveEvent(QMouseEvent *event) {
	if (!hasFocus())
		return;

	bool found = false;
	MarkdownCursor cursor = cursorAtPoint(event->pos(), &found);

	if (found && (cursor != m_cursor)) {
		// Update selection.
		m_selection.active = true;
		m_selection.end = cursor;
	}

	update();
}

void MarkdownEditWidget::mouseReleaseEvent(QMouseEvent *event) {
	if (!hasFocus())
		return;

	bool found = false;
	MarkdownCursor cursor = cursorAtPoint(event->pos(), &found);

	if (!found || (m_selection.start == cursor)) {
		m_selection.active = false;
	}

	if (found) {
		QPoint pos = event->pos();

		// Check if still pressing link.
		if (
			std::abs(pos.x() - m_pressPoint.x()) < 5 &&
			std::abs(pos.y() - m_pressPoint.y()) < 5 &&
			m_anchor.isEmpty() == false
		) {
			onAnchorClicked(m_anchor);
		}
		m_pressPoint = QPoint(0, 0);
		m_anchor = "";

		// Check for double-clicks.
		if (
			m_lastMouseReleaseTime.isValid() &&
			std::abs(pos.x() - m_lastMouseReleasePoint.x()) < 5 &&
			std::abs(pos.y() - m_lastMouseReleasePoint.y()) < 5
		) {
			QTime now = QTime::currentTime();
			if (m_lastMouseReleaseTime.msecsTo(now) <= 300) {
				selectWordUnderCursor();
			}
		}
		m_lastMouseReleaseTime = QTime::currentTime();
		m_lastMouseReleasePoint = pos;
	}

	update();
}

void MarkdownEditWidget::keyPressEvent(QKeyEvent *event) {
	int lineIdx = m_cursor.line;
	MarkdownBlock *block = m_cursor.block;
	MarkdownCursor prev = m_cursor;
	MarkdownCursor cursor = m_cursor;
	int key = event->key();

	if (lineIdx == -1 || block == nullptr)
		return;

	text::Paragraph *par = block->paragraph();
	text::Line *line = &((*par->getLines())[lineIdx]);

	if (
		key == Qt::Key_Escape
	) {
		clearFocus();
		return;
	} else if (
		key == Qt::Key_PageUp ||
		(key == Qt::Key_Up && event->modifiers() & Qt::ControlModifier)
	) {
		if (m_presenter == nullptr)
			return;

		int diff = m_presenter->getPageOffset(false);
		if (diff == 0) {
			cursor = documentStart();
		} else {
			cursor = moveCursor(diff, prev);
			if (cursor.block == nullptr || cursor.line == -1)
				return;
		}

		processCursorMove(prev, cursor);
	} else if (
		key == Qt::Key_PageDown ||
		(key == Qt::Key_Down && event->modifiers() & Qt::ControlModifier)
	) {
		if (m_presenter == nullptr)
			return;

		int diff = m_presenter->getPageOffset(true);
		cursor = moveCursor(diff, prev);
		if (cursor.block == nullptr || cursor.line == -1)
			return;

		processCursorMove(prev, cursor);
	} else if (
		key == Qt::Key_Home ||
		(key == Qt::Key_Left &&
			event->modifiers() & Qt::ControlModifier)
	) {
		if (prev.position == 0) {
			cursor = documentStart();
		} else {
			cursor = MarkdownCursor(prev.block, prev.line, 0);
		}
		processCursorMove(prev, cursor);
	} else if (
		key == Qt::Key_End ||
		(
			key == Qt::Key_Right &&
			event->modifiers() & Qt::ControlModifier
		)
	) {
		if (prev.position == line->text.length()) {
			cursor = documentEnd();
		} else {
			cursor = MarkdownCursor(prev.block, prev.line, line->text.length());
		}
		processCursorMove(prev, cursor);
	} else if (key == Qt::Key_Left) {
		if (m_cursor.position > 0) {
			cursor.position = cursor.position - 1;
			processCursorMove(prev, cursor);
		} else {
			if (
				int prevLineIdx = lineIdx - 1;
				prevLineIdx >= 0
			) {

				cursor.line = prevLineIdx;
				cursor.position = par->getLines()->at(prevLineIdx).folded.length();
				processCursorMove(prev, cursor);
			} else if (
				MarkdownBlock *prevBlock = blockBefore(block);
				prevBlock != nullptr
			) {
				QList<text::Line> *prevLines = prevBlock->paragraph()->getLines();
				cursor.block = prevBlock;
				cursor.line = prevLines->size() - 1;
				cursor.position = par->getLines()->at(cursor.line).folded.length();
				processCursorMove(prev, cursor);
			}
		}
	} else if (key == Qt::Key_Right) {
		if (m_cursor.position < line->text.length()) {
			cursor.position = cursor.position + 1;
			processCursorMove(prev, cursor);
		} else {
			if (
				int nextIdx = lineIdx + 1;
				nextIdx < par->getLines()->size()
			) {
				cursor.line = nextIdx;
				cursor.position = 0;
				processCursorMove(prev, cursor);
			} else if (
				MarkdownBlock *nextBlock = blockAfter(block);
				nextBlock != nullptr
			) {
				QList<text::Line> *nextLines = nextBlock->paragraph()->getLines();
				cursor.block = nextBlock;
				cursor.line = 0;
				cursor.position = 0;
				processCursorMove(prev, cursor);
			}
		}
	} else if (key == Qt::Key_Up) {
		if (
			bool exists = block->cursorAbove(prev, &cursor);
			exists == true
		) {
			processCursorMove(prev, cursor);
		} else if (
			bool exists = cursorAtBlockAbove(prev, &cursor);
			exists == true
		) {
			processCursorMove(prev, cursor);
		}
	} else if (key == Qt::Key_Down) {
		if (
			bool exists = block->cursorBelow(prev, &cursor);
			exists == true
		) {
			processCursorMove(prev, cursor);
		} else if (
			bool exists = cursorAtBlockBelow(prev, &cursor);
			exists == true
		) {
			processCursorMove(prev, cursor);
		}
	} else if (key == Qt::Key_Enter || key == Qt::Key_Return) {
		bool shiftUsed = (event->modifiers() & Qt::ShiftModifier);
		cursor = splitBlocks(cursor, shiftUsed);
		processCursorMove(prev, cursor);
	} else if (key == Qt::Key_Backspace) {
		if (m_selection.active) {
			cursor = deleteSelection();
			processCursorMove(cursor, cursor);
		} else if (cursor.position != 0) {
			QString newText = line->text;
			newText.remove(cursor.position - 1, 1);
			line->setText(newText, par->getType() == text::Code);

			// Update text and adjust cursor.
			cursor.block->setParagraph(par);
			cursor.position -= 1;
			processCursorMove(prev, cursor);
		} else {
			int parIdx = indexOfParagraph(par);
			if (m_blocks.size() > 1 && parIdx > 0) {
				mergeBlocks(parIdx, line, prev);
			}
		}
	} else if (key == Qt::Key_Delete) {
		if (m_selection.active) {
			cursor = deleteSelection();
			processCursorMove(cursor, cursor);
		} else if (cursor.position != line->text.length()) {
			QString newText = line->text;
			newText.remove(cursor.position, 1);
			line->setText(newText, par->getType() == text::Code);

			// Update text and adjust cursor.
			cursor.block->setParagraph(par);
			processCursorMove(prev, cursor);
		} else {
			int parIdx = indexOfParagraph(par);
			if (m_blocks.size() > 1 && parIdx < m_blocks.size() - 1) {
				text::Line *firstLine = &((*(*m_model.paragraphs())[parIdx + 1].getLines())[0]);
				mergeBlocks(parIdx + 1, firstLine, prev);
			}
		}
	} else if (event->matches(QKeySequence::Copy)) {
		if (m_selection.active) {
			copySelectionToClipboard();
		}
	} else if (event->matches(QKeySequence::Paste)) {
		if (m_selection.active) {
			cursor = deleteSelection();
		}
		cursor = pasteFromClipboard();
		processCursorMove(prev, cursor);
	} else if (event->matches(QKeySequence::Cut)) {
		if (m_selection.active) {
			copySelectionToClipboard();
			cursor = deleteSelection();
			processCursorMove(prev, cursor);
		}
	} else if (event->matches(QKeySequence::Bold)) {
		cursor = applyStyleToSelection("**");
		processCursorMove(prev, cursor);
		if (m_selection.active) {
			m_selection.reset();
			update();
		}
	} else if (event->matches(QKeySequence::Italic)) {
		cursor = applyStyleToSelection("*");
		processCursorMove(prev, cursor);
		if (m_selection.active) {
			m_selection.reset();
			update();
		}
	} else if (
		key == Qt::Key_Tab &&
		(par->getType() == text::BulletList || par->getType() == text::NumberList)
	) {
		if (line->level < 4) {
			line->level = line->level + 1;
			block->setParagraph(par);
		}
	} else if (QString text = event->text(); !text.isEmpty()) {
		if (m_selection.active) {
			cursor = deleteSelection();
			prev = cursor;
		}

		text::Paragraph *par = cursor.block->paragraph();
		text::Line *line = &((*par->getLines())[cursor.line]);
		text::ParagraphType type = par->getType();
		QString empty = "";

		static QRegularExpression listExp("^[0-9]+\\. ");
		static QRegularExpression bulletExp("^[\\+\\*\\-] ");

		// Insert text.
		QString newText = line->text;
		newText.insert(cursor.position, text);
		if (newText == "```" && par->getType() != text::Code) {
			// Transform to code code.
			line->setText(empty, true);
			par->setType(text::Code);
			block->setParagraph(par);

			cursor.position = 0;
			processCursorMove(prev, cursor);
		} else if (
			auto m = listExp.match(newText);
			m.hasMatch() && type == text::Text
		) {
			// Transform to numbered list.
			line->setText(empty, false);
			par->setType(text::NumberList);
			block->setParagraph(par);

			cursor.position = 0;
			processCursorMove(prev, cursor);
		} else if (
			auto m = bulletExp.match(newText);
			m.hasMatch() && type == text::Text
		) {
			// Transform to bullet list.
			line->setText(empty, false);
			par->setType(text::BulletList);
			block->setParagraph(par);

			cursor.position = 0;
			processCursorMove(prev, cursor);
		} else {
			// Update text.
			line->setText(newText, par->getType() == text::Code);

			// Update text and adjust cursor.
			cursor.block->setParagraph(par);
			cursor.position += text.length();
			processCursorMove(prev, cursor);
		}
	}

	if (isMovementKey(event)) {
		if (event->modifiers() & Qt::ShiftModifier) {
			m_selection.active = true;
			m_selection.end = m_cursor;
			// Redraw.
			update();
		} else {
			m_selection.active = false;
			m_selection.start = m_cursor;
			m_selection.end = m_cursor;
		}
	} else if (
		!event->matches(QKeySequence::Copy) &&
		!event->text().isEmpty()
	) {
		m_selection.active = false;
		m_selection.start = m_cursor;
		m_selection.start = m_cursor;
	}

	if (!isMovementKey(event)) {
		m_isDirty = true;
		throttleSave();
	}

	// I don't like this. Have to wait for widgets to redraw to avoid
	// crashes on querying non-existent lines in the layout.
	QCoreApplication::processEvents();

	// Return if only mod keys are pressed
	if (
		key == Qt::Key_Control || key == Qt::Key_Alt ||
		key == Qt::Key_Meta || key == Qt::Key_Shift
	) {
		return;
	}

	// Ensure cursor is visible.
	if (m_cursor.block != nullptr && m_cursor.line != -1) {
		QLine cursorLine = m_cursor.block->lineForCursor(m_cursor);
		bool movedUp = (
			key == Qt::Key_Left ||
			key == Qt::Key_Up ||
			key == Qt::Key_Home ||
			key == Qt::Key_PageUp
		);
		// TODO: Really bad stuff. Because of async relayouts of blocks,
		// we have to artificially delay cursor visibility update here.
		// Need to figure out if we can maybe make sure it is done after
		// relayout is complete?
		QTimer::singleShot(100, this, [this, cursorLine, movedUp]{
			emit cursorMoved(cursorLine, movedUp);
		});
	}
}

void MarkdownEditWidget::focusOutEvent(QFocusEvent*) {
	if (m_cursor.block != nullptr) {
		m_lastCursor = m_cursor;
		m_cursor = MarkdownCursor(nullptr, -1, 0);
		update();
	}
}

// Cursor manipulation.

MarkdownCursor MarkdownEditWidget::moveCursor(
	int dy, MarkdownCursor prev
) {
	bool success = false;

	if (prev.block == nullptr) {
		return MarkdownCursor(nullptr, -1, 0);
	}

	MarkdownBlock *block = prev.block;
	QPoint p = block->pointAtCursor(prev);

	QPoint newP = QPoint(p.x(), p.y() + dy);
	MarkdownCursor cursor = MarkdownCursor(nullptr, -1, 0);
	success = cursorAtPoint(newP, &cursor);

	if (success) {
		return cursor;
	} else {
		// If valid position not found, go to start/end of the document
		// instead, depending on the diff direction.
		if (dy > 0) {
			return documentEnd();
		} else if (dy < 0) {
			return documentStart();
		}
	}

	// We should not really get here.
	return cursor;
}

void MarkdownEditWidget::selectWordUnderCursor() {
	if (m_cursor.block == nullptr || m_cursor.line == -1)
		return;

	text::Paragraph *par = m_cursor.block->paragraph();
	text::Line *line = &((*par->getLines())[m_cursor.line]);
	QString text = line->text;

	int start = m_cursor.position, end = m_cursor.position;
	while (start > 0 && text[start - 1].isLetterOrNumber())
		start -= 1;
	while (end < text.length() && text[end].isLetterOrNumber())
		end += 1;

	if (start != end) {
		MarkdownCursor startCursor = MarkdownCursor(m_cursor.block, m_cursor.line, start);
		MarkdownCursor endCursor = MarkdownCursor(m_cursor.block, m_cursor.line, end);

		m_selection = MarkdownSelection(startCursor, endCursor);

		// Redraw.
		processCursorMove(m_cursor, startCursor);
		//update();
	}
}

// Cursor provider.

MarkdownCursor *MarkdownEditWidget::currentCursor() {
	return &m_cursor;
}

QTextLayout::FormatRange MarkdownEditWidget::selectionInLine(
	MarkdownBlock *block,
	text::Line *line
) {
	QTextLayout::FormatRange range = QTextLayout::FormatRange{
		.start = 0,
		.length = 0,
		.format = QTextCharFormat()
	};

	if (m_selection.active == false)
		return range;

	MarkdownCursor startCursor = m_selection.start;
	MarkdownCursor endCursor = m_selection.end;

	if (cursorAfter(startCursor, endCursor))
		std::swap(startCursor, endCursor);

	int curBlockIdx = indexOfParagraph(block->paragraph());
	int curLineIdx = block->paragraph()->indexOfLine(line);

	// Get start and end of the selection.
	int startBlockIdx = indexOfParagraph(startCursor.block->paragraph());
	int startLineIdx = startCursor.line;
	int startPos = startCursor.position;

	int endBlockIdx = indexOfParagraph(endCursor.block->paragraph());
	int endLineIdx = endCursor.line;
	int endPos = endCursor.position;

	// TODO: The logic below looks ugly and confusing. Maybe there's a
	// better way to do all these checks that will come to me when I
	// get some proper sleep for once in my life.

	// Check if current block/line is outside of selection.
	if (
		(curBlockIdx < startBlockIdx) ||
		(curBlockIdx > endBlockIdx) ||
		(curBlockIdx == startBlockIdx && curLineIdx < startLineIdx) ||
		(curBlockIdx == endBlockIdx && curLineIdx > endLineIdx)
	) {
		return range;
	}

	// Check if current line is completely inside selection.
	if (
		(curBlockIdx > startBlockIdx && curBlockIdx < endBlockIdx) ||
		(
			curBlockIdx == startBlockIdx &&
			endBlockIdx != startBlockIdx &&
			curLineIdx > startLineIdx
		) ||
		(
			curBlockIdx == endBlockIdx &&
			endBlockIdx != startBlockIdx &&
			curLineIdx < endLineIdx
		)
	) {
		range.start = 0;
		range.length = line->text.length();
	} else {
		// If selection starts at current line, save start position.
		if (
			curBlockIdx == startBlockIdx &&
			curLineIdx == startLineIdx
		) {
			range.start = startPos;
		}

		// If selection ends outside of current line, select till the end
		// of the line.
		if (endBlockIdx > curBlockIdx || endLineIdx > curLineIdx) {
			range.length = line->text.length() - range.start;
		// Otherwise, select till the selection end. (d'uh)
		} else if (
			curBlockIdx == endBlockIdx &&
			curLineIdx == endLineIdx
		) {
			range.length = endPos - range.start;
		}
	}

	QTextCharFormat format = QTextCharFormat();
	format.setBackground(m_style->selectionBackColor());
	format.setForeground(m_style->selectionTextColor());
	range.format = format;

	return range;
}

bool MarkdownEditWidget::isDocumentEmpty() {
	QList<text::Paragraph> *pars = m_model.paragraphs();
	if (pars->size() > 1 || (*pars)[0].getType() != text::Text)
		return false;

	QList<text::Line> *lines = (*pars)[0].getLines();
	if (lines->size() > 1 || lines->at(0).text.length() > 0)
		return false;

	return true;
}

// Selections and clipboard.

void MarkdownEditWidget::copySelectionToClipboard() {
	if (m_selection.active == false)
		return;

	MarkdownCursor start = m_selection.start;
	MarkdownCursor end = m_selection.end;
	end = adjustForUnfolding(end, start);

	if (cursorAfter(start, end)) {
		std::swap(start, end);
	}

	int startIdx = indexOfParagraph(start.block->paragraph());
	int endIdx = indexOfParagraph(end.block->paragraph());

	QList<text::Paragraph> result;
	QList<text::Paragraph> *pars = m_model.paragraphs();
	for (int idx = startIdx; idx <= endIdx; idx++) {
		// Original paragraph.
		text::Paragraph *par = &((*pars)[idx]);
		// New paragraph's contents.
		QList<text::Line> newLines;

		QList<text::Line> *lines = par->getLines();
		// Get starting and ending line indexes.
		int startLineIdx = 0, endLineIdx = lines->size() - 1;
		if (idx == startIdx)
			startLineIdx = start.line;
		if (idx == endIdx)
			endLineIdx = end.line;

		for (int lineIdx = startLineIdx; lineIdx <= endLineIdx; lineIdx++) {
			// Original line.
			text::Line *line = &((*lines)[lineIdx]);

			// Get starting and ending position within the line.
			int startPos = 0, endPos = line->text.length();
			if (idx == startIdx && lineIdx == startLineIdx)
				startPos = start.position;
			if (idx == endIdx && lineIdx == endLineIdx)
				endPos = end.position;

			QString newText = line->text.mid(startPos, endPos - startPos);
			text::Line newLine = text::Line(newText, par->getType() == text::Code);
			// Save line copy.
			newLines.push_back(newLine);
		}

		// Save paragraph copy.
		text::Paragraph newPar = text::Paragraph(par->getType(), newLines);
		result.push_back(newPar);
	}

	// Convert resulting paragraphs into text.
	text::TextModel model = text::TextModel(result);
	QString text = model.text();

	// Save resulting text to clipboard
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *data = new QMimeData();
	data->setText(text);
	clipboard->setMimeData(data);
}

MarkdownCursor MarkdownEditWidget::deleteSelection() {
	if (m_selection.active == false)
		return m_cursor;

	MarkdownCursor start = m_selection.start;
	MarkdownCursor end = m_selection.end;
	end = adjustForUnfolding(end, start);

	if (cursorAfter(start, end)) {
		std::swap(start, end);
	}

	int startIdx = indexOfParagraph(start.block->paragraph());
	int endIdx = indexOfParagraph(end.block->paragraph());

	// Delete all paragraphs in between selection start and end.
	if (startIdx != endIdx) {
		for (int i = startIdx + 1; i < endIdx; i++)
			deleteParagraph(startIdx + 1);
		// Readjust end index after deletion.
		endIdx = startIdx + 1;
	}

	QList<text::Paragraph> *pars = m_model.paragraphs();

	text::Paragraph *startPar = &((*pars)[startIdx]);
	int startLineIdx = start.line;
	text::Line *startLine = &((*startPar->getLines())[startLineIdx]);

	text::Paragraph *endPar = &((*pars)[endIdx]);
	int endLineIdx = end.line;
	text::Line *endLine = &((*endPar->getLines())[endLineIdx]);

	// Delete all lines in start paragraph after starting line.
	if (startIdx != endIdx) {
		QList<text::Line> *startLines = startPar->getLines();
		if (startLines->size() > startLineIdx + 1) {
			startLines->remove(
				startLineIdx + 1,
				startLines->size() - startLineIdx - 1
			);
		}
		start.block->setParagraph(startPar);

		// Delete all lines in end paragraph before ending line.
		QList<text::Line> *endLines = endPar->getLines();
		if (endLineIdx > 0) {
			endLines->remove(0, endLineIdx);
		}
		// Readjust end line index after deletion.
		endLineIdx = 0;
		endLine = &((*endPar->getLines())[endLineIdx]);
		// Reload block.
		end.block->setParagraph(endPar);
	} else if (startLineIdx < endLineIdx - 1) {
		// Delete all lines between selection.
		QList<text::Line> *startLines = startPar->getLines();
		startLines->remove(
			startLineIdx + 1,
			endLineIdx - startLineIdx - 1
		);
		// Adjust end line index after deletion.
		endLineIdx = startLineIdx + 1;
		endLine = &((*endPar->getLines())[endLineIdx]);
		// Update paragraph.
		start.block->setParagraph(endPar);
	}

	startLine = &((*startPar->getLines())[startLineIdx]);
	endLine = &((*endPar->getLines())[endLineIdx]);

	if (startLine != endLine) {
		// Delete text from start of selection to end of first line.
		QString newStartText = startLine->text.left(start.position);
		startLine->setText(newStartText, startPar->getType() == text::Code);
		start.block->setParagraph(startPar);

		// Delete text from start of last line to end of selection.
		QString newEndText = endLine->text.right(endLine->text.length() - end.position);
		endLine->setText(newEndText, endPar->getType() == text::Code);
		end.block->setParagraph(endPar);
	} else {
		// Delete the selected text within the line.
		text::Line *line = startLine;
		QString newText =
			line->text.left(start.position)
			+ line->text.right(line->text.length() - end.position);
		line->setText(newText, startPar->getType() == text::Code);
		start.block->setParagraph(startPar);
	}

	// Merge starting and ending lines/paragraphs.
	if (startIdx != endIdx || start.line != end.line) {
		mergeBlocks(endIdx, endLine, start);
	}

	start.line = startLineIdx;
	return start;
}

MarkdownCursor MarkdownEditWidget::pasteFromClipboard() {
	QClipboard *clipboard = QApplication::clipboard();
	QString text = clipboard->text();
	return pasteString(text);
}

MarkdownCursor MarkdownEditWidget::pasteString(QString text) {
	MarkdownCursor cursor = m_cursor;
	QList<text::Paragraph> *pars = m_model.paragraphs();
	QStringList list = text.split("\n");
	text::TextModel model = text::TextModel(list);
	QList<text::Paragraph> *newPars = model.paragraphs();

	if (list.size() == 1) {
		// If we're inserting a single line, just inject it into the
		// current line.
		text::Line *line = &((*m_cursor.block->paragraph()->getLines())[m_cursor.line]);
		QString textBefore = line->text.left(m_cursor.position);
		QString textAfter = line->text.right(
			line->text.length() - m_cursor.position
		);
		int pos = m_cursor.position + text.length();

		QString newText = textBefore
			+ text
			+ textAfter;
		line->setText(newText, m_cursor.block->paragraph()->getType() == text::Code);
		m_cursor.block->setParagraph(m_cursor.block->paragraph());

		cursor = MarkdownCursor(
			m_cursor.block,
			m_cursor.line,
			pos
		);
	} else if (m_cursor.block->paragraph()->getType() == text::Code) {
		// If we're inserting into code, ignore text layout and just put
		// data as is.
		int pos = 0;

		text::Paragraph *par = m_cursor.block->paragraph();
		QList<text::Line> *lines = m_cursor.block->paragraph()->getLines();
		int lineIdx = m_cursor.line;
		text::Line *line = &((*par->getLines())[lineIdx]);

		QString textBefore = line->text.left(m_cursor.position);
		QString textAfter = line->text.right(
			line->text.length() - m_cursor.position
		);

		for (auto listLine = list.begin(); listLine != list.end(); listLine++) {
			if (listLine == list.begin()) {
				QString newText = textBefore + (*listLine);
				line->setText(newText, true);
			} else if (listLine + 1 == list.end()) {
				pos = (*listLine).length();
				QString lastText = *listLine + textAfter;
				lines->insert(lineIdx + 1, text::Line(lastText, true));
				lineIdx += 1;
			} else {
				lines->insert(lineIdx + 1, text::Line(*listLine, true));
				lineIdx += 1;
			}
		}

		m_cursor.block->setParagraph(m_cursor.block->paragraph());
		cursor = MarkdownCursor(
			m_cursor.block,
			lineIdx,
			pos
		);
	} else {
		MarkdownBlock *block = cursor.block;
		text::Paragraph *par = block->paragraph();
		text::Line *line = &((*par->getLines())[cursor.line]);
		int parIdx = indexOfParagraph(par);
		int lineIdx = cursor.line;

		// Split current position as if we've pressed Return key.
		if (!line->text.isEmpty())
			splitBlocks(cursor, false);

		if (par->getType() != text::Text) {
			QList<text::Line> *lines = par->getLines();
			// Copy lines after current one to a new paragraph.
			QList<text::Line> remainder;
			for (int i = lineIdx + 1; i < lines->size(); i++) {
				remainder.push_back((*lines)[i]);
			}

			// Remove lines after current one from old paragraph.
			lines->remove(lineIdx + 1, lines->size() - lineIdx - 1);

			// Create a new paragraph with the remainder.
			if (remainder.size() > 0) {
				text::Paragraph newPar = text::Paragraph(
					par->getType(),
					remainder
				);
				text::Paragraph *ptr = insertParagraph(parIdx + 1, newPar);
			}

			// Update old paragraph.
			block->setParagraph(par);
		}

		// Insert new paragraphs below current one.
		text::Paragraph *lastPar = nullptr;
		for (int idx = 0; idx < newPars->size(); idx++) {
			lastPar = insertParagraph(parIdx + 1, (*newPars)[idx]);
			parIdx += 1;
		}

		QList<text::Line> *lastLines = lastPar->getLines();
		text::Line *lastLine = &((*lastLines)[lastLines->size() - 1]);
		cursor = MarkdownCursor(
			m_blocks[indexOfParagraph(lastPar)],
			lastLines->size() - 1,
			lastLine->text.length()
		);
	}

	return cursor;
}

// Search.

void MarkdownEditWidget::showSearchWidget(
	QWidget *widget,
	QPoint belowPoint
) {
	assert(widget != nullptr);
	m_search = widget;

	// Calculate position.
	QSize hint = widget->sizeHint();
	QSize cur = size();
	int width = std::min(cur.width(), 400);
	int x = std::max(belowPoint.x() - width / 2, 0);

	widget->setMinimumWidth(width);
	widget->setGeometry(
		x, belowPoint.y() + 5,
		width, hint.height()
	);

	widget->setParent(this);
	widget->raise();
	widget->show();
	widget->setFocus();
}

void MarkdownEditWidget::hideSearchWidget() {
	if (m_search == nullptr)
		return;

	m_search->deleteLater();
	m_search = nullptr;
}

void MarkdownEditWidget::insertNodeLink(ThoughtId id, QString title) {
	QString linkName = title;

	// If we have a selection, put it as a link title.
	if (m_selection.active) {
		// But only if the selection is within single block.
		if (m_selection.start.line == m_selection.end.line) {
			text::Line line = m_selection.start.block
				->paragraph()
				->getLines()
				->at(m_selection.start.line);
			linkName = line.text.mid(
				m_selection.start.position,
				m_selection.end.position - m_selection.start.position
			);
		}
	}

	if (m_selection.active) {
		deleteSelection();
	}

	if (m_lastCursor.block != nullptr) {
		m_cursor = m_lastCursor;
		setFocus();
	}

	QString data = QString("[%1](node://%2)")
		.arg(linkName)
		.arg(id);
	MarkdownCursor cursor = pasteString(data);

	m_selection.reset();

	processCursorMove(m_cursor, cursor);

	// Schedule save.
	m_isDirty = true;
	throttleSave();
}

// Context menu.

void MarkdownEditWidget::showContextMenu(QMouseEvent *event) {
	QMenu *menu = new QMenu();
	menu->setStyleSheet(
		QString("QMenu{\
			background-color: %1;\
		}\
		QMenu::item{\
			padding: 2px 25px 2px 20px;\
			color: %2;\
			font: normal %3px \"%4\";\
			background-color: %5;\
		}\
		QMenu::item:hover{\
			background-color: %6;\
		}\
		QMenu::item:selected{\
			background-color: %7;\
		}")
		.arg(m_style->background().name(QColor::HexRgb))
		.arg(m_style->textColor().name(QColor::HexRgb))
		.arg(m_style->font().pixelSize())
		.arg(m_style->font().family())
		.arg(m_style->background().name(QColor::HexRgb))
		.arg(m_style->background().lighter(150).name(QColor::HexRgb))
		.arg(m_style->background().lighter(200).name(QColor::HexRgb))
	);

	// Custom node link action.
	QString connectMenu = tr("Connect thought...");
	QAction *action = new QAction(connectMenu, this);
	connect(
		action, SIGNAL(triggered()),
		this, SLOT(onInsertNodeLink())
	);
	menu->insertAction(nullptr, action);

	// Show the menu.
	menu->exec(mapToGlobal(event->pos()));
}

// Linking nodes.

void MarkdownEditWidget::onInsertNodeLink() {
	if (m_lastCursor.block != nullptr) {
		m_cursor = m_lastCursor;
	}

	QLine cursorLine = m_cursor.block->lineForCursor(m_cursor);
	emit nodeInsertionActivated(cursorLine.p2());
}

// Saving.

void MarkdownEditWidget::throttleSave() {
	if (!m_isDirty)
		return;

	if (m_saveTimer != nullptr)
		return;

	m_saveTimer = new QTimer(this);
	QObject::connect(
		m_saveTimer, SIGNAL(timeout()),
		this, SLOT(saveText())
	);
	m_saveTimer->setSingleShot(true);
	m_saveTimer->start(10000);
}

void MarkdownEditWidget::saveText() {
	if (!m_isDirty)
		return;

	// Select all text.
	QString txt = m_model.text();

	// Emit text change event.
	emit textChanged(txt);

	// Reset dirty flag.
	m_isDirty = false;
	// Delete timer.
	if (m_saveTimer != nullptr) {
		delete m_saveTimer;
		m_saveTimer = nullptr;
	}
}

// Helpers

MarkdownBlock *MarkdownEditWidget::blockBefore(MarkdownBlock *block) {
	for (int idx = 1; idx < m_blocks.size(); idx++) {
		if (&(*m_blocks[idx]) == block)
			return &(*m_blocks[idx-1]);
	}

	return nullptr;
}

MarkdownBlock *MarkdownEditWidget::blockAfter(MarkdownBlock *block) {
	for (int idx = 0; idx < m_blocks.size() - 1; idx++) {
		if (&(*m_blocks[idx]) == block)
			return &(*m_blocks[idx+1]);
	}

	return nullptr;
}

bool MarkdownEditWidget::cursorAtBlockBelow(
	MarkdownCursor cur,
	MarkdownCursor *result
) {
	if (cur.block == nullptr)
		return false;

	MarkdownBlock *next = blockAfter(cur.block);
	if (next == nullptr)
		return false;

	qreal curX = cur.block->xAtCursor(cur);
	MarkdownCursor nextCursor = next->firstCursorAtX(curX);
	*result = nextCursor;
	return true;
}

bool MarkdownEditWidget::cursorAtBlockAbove(
	MarkdownCursor cur,
	MarkdownCursor *result
) {
	if (cur.block == nullptr)
		return false;

	MarkdownBlock *prev = blockBefore(cur.block);
	if (prev == nullptr)
		return false;

	qreal curX = cur.block->xAtCursor(cur);
	MarkdownCursor prevCursor = prev->lastCursorAtX(curX);
	*result = prevCursor;
	return true;
}

bool MarkdownEditWidget::cursorAbovePoint(
	QPoint pos,
	MarkdownCursor *result
) {
	for (auto it = m_blocks.rbegin(); it != m_blocks.rend(); it++) {
		QRect geometry = (*it)->geometry();

		// Since there are gaps between blocks, we take the first block
		// that has the point inside or is located below the point.
		if (geometry.y() + geometry.height() > pos.y()) {
			continue;
		}

		QList<text::Line> *lines = (*it)->paragraph()->getLines();
		text::Line *lastLine = &((*lines)[lines->size() - 1]);
		*result = MarkdownCursor(
			(*it),
			lines->size() - 1,
			lastLine->text.length()
		);
		return true;
	}

	return false;
}

void MarkdownEditWidget::processCursorMove(
	MarkdownCursor from,
	MarkdownCursor to
) {
	int offset = 0, pos = to.position;

	// Adjust cursor position for unfolded text.
	if (
		(from.block != to.block || from.line != to.line)
	) {
		text::Line *line = &((*to.block->paragraph()->getLines())[to.line]);
		QList<text::FormatRange> *ranges = &line->foldedFormats;

		for (auto& range: *ranges) {
			if (range.to < pos) {
				offset += range.endOffset();
			}
			if (range.from < pos) {
				offset += range.startOffset();
			}
		}

		to.position = pos + offset;
	}

	m_cursor = to;
	emit onCursorMove(from, to);
}

MarkdownCursor MarkdownEditWidget::adjustForUnfolding(
	MarkdownCursor cursor,
	MarkdownCursor from
) {
	MarkdownCursor to = cursor;
	int offset = 0, pos = to.position;

	if (to.line == -1)
		return cursor;

	if (
		(from.block != to.block || from.line != to.line)
	) {
		text::Line *line = &((*to.block->paragraph()->getLines())[to.line]);
		QList<text::FormatRange> *ranges = &line->foldedFormats;

		for (auto& range: *ranges) {
			if (range.to <= pos) {
				offset += range.endOffset();
			}
			if (range.from < pos) {
				offset += range.startOffset();
			}
		}

		to.position = pos + offset;
	}

	return to;
}

MarkdownCursor MarkdownEditWidget::applyStyleToSelection(
	QString style
) {
	MarkdownCursor startCursor = m_cursor;
	MarkdownCursor endCursor = m_cursor;

	if (m_selection.active) {
		startCursor = m_selection.start;
		endCursor = m_selection.end;
		if (cursorAfter(startCursor, endCursor))
			std::swap(startCursor, endCursor);
	}

	// Adjust for unfolding.
	startCursor = adjustForUnfolding(startCursor, m_cursor);
	endCursor = adjustForUnfolding(endCursor, m_cursor);

	// Add bold mark before selection.
	text::Paragraph *startPar = startCursor.block->paragraph();
	text::Line *startLine = &((*startPar->getLines())[startCursor.line]);
	QString newText = startLine->text;
	newText.insert(startCursor.position, style);
	startLine->setText(newText, startPar->getType() == text::Code);

	// Add bold mark after selection.
	text::Paragraph *endPar = endCursor.block->paragraph();
	text::Line *endLine = &((*endPar->getLines())[endCursor.line]);
	if (endLine == startLine)
		endCursor.position += style.length();
	newText = endLine->text;
	newText.insert(endCursor.position, style);
	endLine->setText(newText, endPar->getType() == text::Code);

	// Update paragraphs.
	startCursor.block->setParagraph(startPar);
	if (endCursor.block != startCursor.block)
		endCursor.block->setParagraph(endPar);

	// Put cursor at the end of the selection if there is a selection.
	if (m_selection.active)
		endCursor.position += style.length();

	return endCursor;
}

bool MarkdownEditWidget::cursorAtPoint(
	QPoint pos,
	MarkdownCursor *cursor
) {
	for (auto it = m_blocks.begin(); it != m_blocks.end(); it++) {
		QRect geometry = (*it)->geometry();

		// Since there are gaps between blocks, we take the first block
		// that has the point inside or is located below the point.
		if (geometry.y() + geometry.height() < pos.y()) {
			continue;
		}

		// Translate inside block.
		QPoint adjusted = pos;
		if (geometry.y() > pos.y())
			adjusted = QPoint(pos.x(), geometry.y());

		QPoint pointInside = (*it)->mapFromParent(adjusted);
		if (bool found = (*it)->cursorAt(pointInside, cursor); found == true) {
			return true;
		}
	}

	return false;
}

MarkdownCursor MarkdownEditWidget::documentStart() {
	MarkdownBlock *block = m_blocks[0];
	return MarkdownCursor(block, 0, 0);
}

MarkdownCursor MarkdownEditWidget::documentEnd() {
	MarkdownBlock *block = m_blocks[m_blocks.size() - 1];
	text::Paragraph *par = block->paragraph();
	QList<text::Line> *lines = par->getLines();
	text::Line *line = &((*lines)[lines->size() - 1]);
	return MarkdownCursor(block, lines->size() - 1, line->folded.length());
}

inline int MarkdownEditWidget::indexOfParagraph(text::Paragraph *par) {
	QList<text::Paragraph> *pars = m_model.paragraphs();
	for (int idx = 0; idx < pars->size(); idx++) {
		if (&((*pars)[idx]) == par)
			return idx;
	}

	return -1;
}

inline text::Paragraph *MarkdownEditWidget::insertParagraph(
	int index,
	text::Paragraph par
) {
	m_model.insert(index, par);

	MarkdownBlock *block = new MarkdownBlock(nullptr, m_style, this);
	connect(
		this, &MarkdownEditWidget::onCursorMove,
		block, &MarkdownBlock::onCursorMove
	);

	block->setParagraph(&((*m_model.paragraphs())[index]));
	m_blocks.insert(index, block);
	m_layout->insertWidget(index, block);

	for (int idx = 0; idx < m_model.paragraphs()->size(); idx++) {
		m_blocks[idx]->updateParagraphWithoutReload(m_model.paragraphs()->data() + idx);
	}

	m_layout->invalidate();
	return &((*m_model.paragraphs())[index]);
}

inline void MarkdownEditWidget::deleteParagraph(int index) {
	MarkdownBlock *block = m_blocks[index];
	block->updateParagraphWithoutReload(nullptr);

	disconnect(
		this, &MarkdownEditWidget::onCursorMove,
		block, &MarkdownBlock::onCursorMove
	);

	// Delete from model.
	m_model.paragraphs()->remove(index, 1);

	// Delete from list of widgets.
	m_blocks.remove(index, 1);

	// Delete from layout.
	m_layout->removeWidget(block);

	// Delete the object.
	block->deleteLater();

	// Update paragraph pointers for all blocks after new one.
	QList<text::Paragraph> *pars = m_model.paragraphs();
	for (int idx = 0; idx < pars->size(); idx++) {
		m_blocks[idx]->updateParagraphWithoutReload(&((*pars)[idx]));
	}
}

void MarkdownEditWidget::mergeBlocks(
	int next,
	text::Line *line,
	MarkdownCursor prev
) {
	MarkdownCursor cursor = prev;
	QList<text::Paragraph> *pars = m_model.paragraphs();
	text::Paragraph *par = &((*pars)[next]);
	MarkdownBlock *block = m_blocks[next];

	if (par->getType() == text::Text) {
		int parIdx = indexOfParagraph(par);
		if (parIdx > 0) {
			// Get last line of the previous paragraph.
			text::Paragraph *prevPar = &((*m_model.paragraphs())[parIdx - 1]);
			MarkdownBlock *prevBlock = m_blocks[parIdx - 1];
			QList<text::Line> *lines = prevPar->getLines();
			text::Line *lastLine = &((*lines)[lines->size() - 1]);

			// Append current text to the last line.
			int newPosition = lastLine->folded.length();
			QString newText = lastLine->text + line->text;
			lastLine->setText(newText, prevPar->getType() == text::Code);

			// Update previous block.
			prevBlock->setParagraph(prevPar);

			// Delete current paragraph.
			deleteParagraph(parIdx);

			// Update cursor.
			cursor = MarkdownCursor(
				prevBlock,
				lines->size() - 1,
				newPosition
			);
			processCursorMove(prev, cursor);
		}
	} else {
		int parIdx = indexOfParagraph(par);
		int lineIdx = par->indexOfLine(line);

		if (
			(par->getType() == text::BulletList || par->getType() == text::NumberList) &&
			line->level > 0
		) {
			line->level = line->level - 1;
			block->setParagraph(par);
		} else if (lineIdx == 0) {
			QList<text::Paragraph> *pars = m_model.paragraphs();
			if (
				parIdx > 0 &&
				(*pars)[parIdx - 1].getType() == par->getType()
			) {
				text::Paragraph *prevPar = &((*pars)[parIdx - 1]);
				MarkdownBlock *prevBlock = m_blocks[parIdx - 1];
				QList<text::Line> *prevLines = prevPar->getLines();
				QList<text::Line> *lines = par->getLines();

				// Copy current paragraph lines to previous one.
				int oldSize = prevLines->size();
				for (int idx = 0; idx < lines->size(); idx++) {
					prevLines->push_back((*lines)[idx]);
				}

				// Delete current paragraph.
				deleteParagraph(parIdx);

				// Update previous paragraph.
				prevBlock->setParagraph(prevPar);

				// Update cursor to last line of previous paragraph.
				cursor = MarkdownCursor(
					prevBlock,
					oldSize,
					0
				);
				processCursorMove(prev, cursor);
			} else if (parIdx > 0) {
				text::Paragraph *prevPar = &((*pars)[parIdx - 1]);
				MarkdownBlock *prevBlock = m_blocks[parIdx - 1];
				QList<text::Line> *lines = par->getLines();
				QList<text::Line> *prevLines = prevPar->getLines();
				text::Line *lastLine = &((*prevLines)[prevLines->size() - 1]);

				// Update previous paragraph's text.
				QString newText = lastLine->text + line->text;
				int newPos = lastLine->folded.length();
				lastLine->setText(newText, prevPar->getType() == text::Code);

				// Delete current line, or the whole paragraph if it has a
				// single line.
				if (lines->size() == 1) {
					deleteParagraph(parIdx);
					prevPar = &((*m_model.paragraphs())[parIdx - 1]);
					prevBlock->setParagraph(prevPar);
				} else {
					lines->remove(lineIdx);
					prevLines = prevPar->getLines();
					lastLine = &((*prevLines)[prevLines->size() - 1]);
					// Update blocks.
					block->setParagraph(par);
				}

				// Update cursor to previous line.
				cursor = MarkdownCursor(
					prevBlock,
					prevLines->size() - 1,
					newPos
				);
				processCursorMove(prev, cursor);
			}
		} else {
			// Merge current and previous lines.
			QList<text::Line> *lines = par->getLines();
			text::Line *prevLine = &((*lines)[lineIdx - 1]);
			QString newText = prevLine->text + line->text;
			int newPos = prevLine->folded.length();
			prevLine->setText(newText, par->getType() == text::Code);

			// Delete current line.
			lines->remove(lineIdx);
			lines = par->getLines();
			prevLine = &((*lines)[lineIdx - 1]);

			// Update block.
			block->setParagraph(par);

			// Update cursor to previous line.
			cursor = MarkdownCursor(
				block,
				lineIdx - 1,
				newPos
			);
			processCursorMove(prev, cursor);
		}
	}
}

MarkdownCursor MarkdownEditWidget::splitBlocks(
	MarkdownCursor cursor,
	bool shiftUsed
) {
	MarkdownBlock *block = cursor.block;
	text::Paragraph *par = block->paragraph();
	int lineIdx = cursor.line;
	text::Line *line = &((*par->getLines())[lineIdx]);
	int parIdx = indexOfParagraph(par);

	QString beforeText = line->text.left(cursor.position);
	QString afterText = line->text.right(line->text.length() - cursor.position);

	if (par->getType() == text::Text) {
		// Insert new paragraph.
		line->setText(beforeText, false);
		text::Paragraph newPar = text::Paragraph(
			text::Text,
			text::Line(afterText, false)
		);
		text::Paragraph *ptr = insertParagraph(parIdx + 1, newPar);

		// Reload current paragraph.
		block->setParagraph(&((*m_model.paragraphs())[parIdx]));

		// Update cursor.
		return MarkdownCursor(m_blocks[parIdx + 1], 0, 0);
	} else {
		QList<text::Line> *lines = par->getLines();

		if (
			line->text.isEmpty() &&
			!shiftUsed &&
			(par->getType() != text::Code || lineIdx == 0 || lineIdx == lines->size() - 1)
		) {
			// Copy lines after current one to a new paragraph.
			QList<text::Line> remainder;
			for (int i = lineIdx + 1; i < lines->size(); i++) {
				remainder.push_back((*lines)[i]);
			}

			// Remove lines after current one from old paragraph.
			lines->remove(lineIdx, lines->size() - lineIdx);
			// Create a new paragraph with the remainder.
			if (remainder.size() > 0) {
				text::Paragraph newPar = text::Paragraph(
					par->getType(),
					remainder
				);
				text::Paragraph *ptr = insertParagraph(parIdx + 1, newPar);
				par = m_model.paragraphs()->data() + parIdx;
				lines = par->getLines();
			}

			QString empty = "";
			if (lines->size() > 0) {
				// Update old paragraph.
				block->setParagraph(par);

				// Insert new empty paragraph.
				text::Paragraph newPar = text::Paragraph(
					text::Text, text::Line(empty, false)
				);
				text::Paragraph *ptr = insertParagraph(parIdx + 1, newPar);

				// Update cursor to new paragraph.
				cursor = MarkdownCursor(m_blocks[parIdx + 1], 0, 0);
			} else {
				par->setType(text::Text);
				lines->push_back(text::Line(empty, false));
				block->setParagraph(par);

				// Update cursor to new current empty line.
				cursor = MarkdownCursor(m_blocks[parIdx], 0, 0);
			}

			return cursor;
		} else {
			QList<text::Line> *lines = par->getLines();
			line->setText(beforeText, par->getType() == text::Code);

			// Insert new line.
			lines->insert(
				lineIdx + 1,
				text::Line(afterText, par->getType() == text::Code, line->level)
			);

			// Reload current paragraph.
			block->setParagraph(par);

			// Update cursor to new line.
			return MarkdownCursor(
				m_blocks[parIdx],
				lineIdx + 1,
				0
			);
		}
	}
}

MarkdownCursor MarkdownEditWidget::cursorAtPoint(
	QPoint pos,
	bool *success
) {
	MarkdownCursor cursor(nullptr, -1, 0);
	bool found = false;

	for (auto it = m_blocks.begin(); it != m_blocks.end(); it++) {
		QRect geometry = (*it)->geometry();
		if (geometry.y() > pos.y() || geometry.y() + geometry.height() < pos.y())
			continue;

		QPoint pointInside = (*it)->mapFromParent(pos);
		if (found = (*it)->cursorAt(pointInside, &cursor); found == true) {
			break;
		}
	}

	*success = found;
	return cursor;
}

bool MarkdownEditWidget::isMovementKey(QKeyEvent *event) {
	int key = event->key();
	switch (key) {
		case Qt::Key_PageUp:
			return true;
		case Qt::Key_PageDown:
			return true;
		case Qt::Key_Home:
			return true;
		case Qt::Key_End:
			return true;
		case Qt::Key_Down:
			return true;
		case Qt::Key_Left:
			return true;
		case Qt::Key_Up:
			return true;
		case Qt::Key_Right:
			return true;
		default:
			return false;
	}
}

bool MarkdownEditWidget::cursorAfter(
	MarkdownCursor first, MarkdownCursor second
) {
	text::Paragraph *firstPar = first.block->paragraph();
	int firstParIdx = indexOfParagraph(firstPar);
	text::Paragraph *secondPar = second.block->paragraph();
	int secondParIdx = indexOfParagraph(secondPar);

	if (firstParIdx < secondParIdx)
		return false;
	if (firstParIdx > secondParIdx)
		return true;

	int firstLineIdx = first.line;
	int secondLineIdx = second.line;

	if (firstLineIdx < secondLineIdx)
		return false;
	if (firstLineIdx > secondLineIdx)
		return true;

	return (first.position > second.position);
}

void MarkdownEditWidget::checkForLinksUnderCursor(MarkdownCursor cur) {
	text::Line *line = nullptr;
	QString anchor = "";
	int position = cur.position;
	QList<text::FormatRange> ranges;
	QString text;

	if (cur.line == -1 || cur.line >= cur.block->paragraph()->getLines()->size())
		return;

	line = &((*cur.block->paragraph()->getLines())[cur.line]);

	if (m_cursor.block == cur.block && m_cursor.line == cur.line) {
		ranges = line->formats;
		text = line->text;
	} else {
		ranges = line->foldedFormats;
		text = line->folded;
	}

	for (auto& fmt: ranges) {
		if (
			fmt.format != text::Link &&
			fmt.format != text::PlainLink &&
			fmt.format != text::NodeLink
		) {
			continue;
		}

		if (!(fmt.from < position && fmt.to > position))
			continue;
		if (fmt.link.target.isEmpty())
			continue;
		anchor = fmt.link.target;
		break;
	}

	if (!anchor.isEmpty()) {
		m_anchor = anchor;
	} else {
		m_anchor = "";
	}
}

void MarkdownEditWidget::onAnchorClicked(QString anchor) {
	static QRegularExpression expr("(.+)://(.+)");

	if (!anchor.isEmpty()) {
		QRegularExpressionMatch match = expr.match(m_anchor);
		if (match.hasMatch()) {
			QString scheme = match.captured(1);
			if (scheme == "node") {
				QString value = match.captured(2);
				bool success = false;
				ThoughtId thoughtId = value.toULongLong(&success);
				if (success) {
					emit nodeLinkSelected(thoughtId);
				}
			} else {
				QDesktopServices::openUrl(QUrl(anchor));
			}
		}
	}
}

