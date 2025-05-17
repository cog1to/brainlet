#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QColor>
#include <QMouseEvent>
#include <QKeyEvent>

#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/markdown_block_widget.h"
#include "widgets/markdown_edit_widget.h"
#include "model/new_text_model.h"

MarkdownEditWidget::MarkdownEditWidget(QWidget *widget, Style *style)
	: BaseWidget(widget, style),
	m_cursor(nullptr, nullptr, 0)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	setStyleSheet(
		QString("background-color: %1")
			.arg(style->background().name(QColor::HexRgb))
	);

	m_layout = new QVBoxLayout(nullptr);
	setLayout(m_layout);
}

MarkdownEditWidget::~MarkdownEditWidget() {
	for (auto widget: m_blocks)
		delete widget;
}

void MarkdownEditWidget::load(QString data) {
	QRegularExpression splitExp(
		"(\\n|\r\\n)", QRegularExpression::MultilineOption
	);
	QStringList lines = data.split(splitExp);
	m_model = text::TextModel(lines);

	// Create blocks.
	QList<text::Paragraph> *list = m_model.paragraphs();

	for (qsizetype i = 0; i < list->size(); ++i) {
		text::Paragraph *p = &((*list)[i]);
		qDebug() << "item type:" << p->getType();

		QList<text::Line> *lines = p->getLines();
		for (qsizetype j = 0; j < lines->size(); j++) {
			qDebug() << "line:" << lines->at(j).text;
		}

		MarkdownBlock *block = new MarkdownBlock(nullptr, m_style, this);
		connect(
			this, &MarkdownEditWidget::onCursorMove,
			block, &MarkdownBlock::onCursorMove
		);

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

// Events.

void MarkdownEditWidget::resizeEvent(QResizeEvent *event) {
	BaseWidget::resizeEvent(event);
}

void MarkdownEditWidget::mousePressEvent(QMouseEvent *event) {
	MarkdownCursor cursor(nullptr, nullptr, 0);
	bool found = false;

	for (auto it = m_blocks.begin(); it != m_blocks.end(); it++) {
		if ((*it)->geometry().contains(event->pos()) == false)
			continue;

		QPoint pointInside = (*it)->mapFromParent(event->pos());
		if (found = (*it)->cursorAt(pointInside, &cursor); found == true) {
			break;
		}
	}

	if (found) {
		MarkdownCursor prev = m_cursor;
		processCursorMove(prev, cursor);
		setFocus();
	} else {
		clearFocus();
	}
}

void MarkdownEditWidget::keyPressEvent(QKeyEvent *event) {
	text::Line *line = m_cursor.line;
	MarkdownBlock *block = m_cursor.block;
	MarkdownCursor prev = m_cursor;
	MarkdownCursor cursor = m_cursor;
	int key = event->key();

	if (line == nullptr || block == nullptr)
		return;

	text::Paragraph *par = block->paragraph();

	if (
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
			if (cursor.block == nullptr || cursor.line == nullptr)
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
		if (cursor.block == nullptr || cursor.line == nullptr)
			return;

		processCursorMove(prev, cursor);
	} else if (
		key == Qt::Key_Home ||
		(key == Qt::Key_Left &&
		 event->modifiers() & Qt::ControlModifier)
	) {
		cursor = documentStart();
		processCursorMove(prev, cursor);
	} else if (
		key == Qt::Key_End ||
		(key == Qt::Key_Right &&
		 event->modifiers() & Qt::ControlModifier)
	) {
		cursor = documentEnd();
		processCursorMove(prev, cursor);
	} else if (key == Qt::Key_Left) {
		if (m_cursor.position > 0) {
			cursor.position = cursor.position - 1;
			processCursorMove(prev, cursor);
		} else {
			if (
				text::Line *prevLine = block->lineBefore(line);
				prevLine != nullptr
			) {
				cursor.line = prevLine;
				cursor.position = prevLine->folded.length();
				processCursorMove(prev, cursor);
			} else if (
				MarkdownBlock *prevBlock = blockBefore(block);
				prevBlock != nullptr
			) {
				QList<text::Line> *prevLines = prevBlock->paragraph()->getLines();
				cursor.block = prevBlock;
				cursor.line = &((*prevLines)[prevLines->size() - 1]);
				cursor.position = cursor.line->folded.length();
				processCursorMove(prev, cursor);
			}
		}
	} else if (key == Qt::Key_Right) {
		if (m_cursor.position < line->text.length()) {
			cursor.position = cursor.position + 1;
			processCursorMove(prev, cursor);
		} else {
			if (
				text::Line *nextLine = block->lineAfter(line);
				nextLine != nullptr
			) {
				cursor.line = nextLine;
				cursor.position = 0;
				processCursorMove(prev, cursor);
			} else if (
				MarkdownBlock *nextBlock = blockAfter(block);
				nextBlock != nullptr
			) {
				QList<text::Line> *nextLines = nextBlock->paragraph()->getLines();
				cursor.block = nextBlock;
				cursor.line = &((*nextLines)[0]);
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
		QString beforeText = line->text.left(cursor.position);
		QString afterText = line->text.right(line->text.length() - cursor.position);
		int parIdx = indexOfParagraph(par);

		if (par->getType() == text::Text) {
			// Insert new paragraph.
			line->setText(beforeText, false);
			text::Paragraph newPar = text::Paragraph(
				text::Text,
				text::Line(afterText, false)
			);
			text::Paragraph *ptr = insertParagraph(parIdx + 1, newPar);

			// Reload current paragraph.
			block->setParagraph(par);

			// Update cursor.
			cursor = MarkdownCursor(
				m_blocks[parIdx + 1],
				&((*ptr->getLines())[0]),
				0
			);
			processCursorMove(prev, cursor);
		} else {
			QList<text::Line> *lines = par->getLines();
			int lineIdx = par->indexOfLine(line);

			if (
				line->text.isEmpty() &&
				((event->modifiers() & Qt::ShiftModifier) == 0) &&
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
					cursor = MarkdownCursor(
						m_blocks[parIdx + 1],
						&((*ptr->getLines())[0]),
						0
					);
				} else {
					par->setType(text::Text);
					lines->push_back(text::Line(empty, false));
					block->setParagraph(par);

					// Update cursor to new current empty line.
					cursor = MarkdownCursor(
						m_blocks[parIdx],
						&((*par->getLines())[0]),
						0
					);
				}

				processCursorMove(prev, cursor);
			} else {
				int lineIdx = par->indexOfLine(line);
				QList<text::Line> *lines = par->getLines();
				line->setText(beforeText, par->getType() == text::Code);

				// Insert new line.
				lines->insert(
					lineIdx + 1,
					text::Line(afterText, par->getType() == text::Code)
				);

				// Reload current paragraph.
				block->setParagraph(par);

				// Update cursor to new line.
				cursor = MarkdownCursor(
					m_blocks[parIdx],	
					&((*lines)[lineIdx + 1]),
					0
				);
				processCursorMove(prev, cursor);
			}
		}
	} else if (key == Qt::Key_Backspace) {
		if (cursor.position != 0) {
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
		if (cursor.position != cursor.line->text.length()) {
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
	} else if (QString text = event->text(); !text.isEmpty()) {
		text::Paragraph *par = cursor.block->paragraph();
		text::Line *line = cursor.line;

		// Insert text.
		QString newText = line->text;
		newText.insert(cursor.position, text);
		line->setText(newText, par->getType() == text::Code);

		// Update text and adjust cursor.
		cursor.block->setParagraph(par);
		cursor.position += text.length();
		processCursorMove(prev, cursor);
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
	if (m_cursor.block != nullptr && m_cursor.line != nullptr) {
		QLine cursorLine = m_cursor.block->lineForCursor(m_cursor);
		emit cursorMoved(cursorLine);
	}
}

// Cursor manipulation.

MarkdownCursor MarkdownEditWidget::moveCursor(
	int dy, MarkdownCursor prev
) {
	bool success = false;

	if (prev.block == nullptr) {
		return MarkdownCursor(nullptr, nullptr, 0);
	}

	MarkdownBlock *block = prev.block;
	QPoint p = block->pointAtCursor(prev);

	QPoint newP = QPoint(p.x(), p.y() + dy);
	MarkdownCursor cursor = MarkdownCursor(nullptr, nullptr, 0);
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

// Cursor provider.

MarkdownCursor *MarkdownEditWidget::currentCursor() {
	return &m_cursor;
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

void MarkdownEditWidget::processCursorMove(
	MarkdownCursor from,
	MarkdownCursor to
) {
	int offset = 0, pos = to.position;

	// Adjust cursor position for unfolded text.
	if (
		(from.block != to.block || (*from.line) != (*to.line))
	) {
		QList<text::FormatRange> *ranges = &to.line->foldedFormats;

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
	text::Paragraph *par = block->paragraph();
	text::Line *line = &((*par->getLines())[0]);
	return MarkdownCursor(block, line, 0);
}

MarkdownCursor MarkdownEditWidget::documentEnd() {
	MarkdownBlock *block = m_blocks[m_blocks.size() - 1];
	text::Paragraph *par = block->paragraph();
	QList<text::Line> *lines = par->getLines();
	text::Line *line = &((*lines)[lines->size() - 1]);
	return MarkdownCursor(block, line, line->folded.length());
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
	QList<text::Paragraph> *pars = m_model.paragraphs();
	pars->insert(index, par);

	MarkdownBlock *block = new MarkdownBlock(nullptr, m_style, this);
	connect(
		this, &MarkdownEditWidget::onCursorMove,
		block, &MarkdownBlock::onCursorMove
	);

	block->setParagraph(&((*pars)[index]));
	m_blocks.insert(index, block);
	m_layout->insertWidget(index, block);

	// Update paragraph pointers for all blocks after new one.
	for (int idx = index + 1; idx < pars->size(); idx++) {
		m_blocks[idx]->updateParagraphWithoutReload(&((*pars)[idx]));
	}

	return &((*pars)[index]);
}

inline void MarkdownEditWidget::deleteParagraph(int index) {
	MarkdownBlock *block = m_blocks[index];
	QList<text::Paragraph> *pars = m_model.paragraphs();

	disconnect(
		this, &MarkdownEditWidget::onCursorMove,
		block, &MarkdownBlock::onCursorMove
	);

	// Delete from model.
	pars->remove(index, 1);

	// Delete from list of widgets.
	m_blocks.remove(index, 1);

	// Delete from layout.
	m_layout->removeWidget(block);

	// Delete the object.
	block->deleteLater();

	// Update paragraph pointers for all blocks after new one.
	for (int idx = index; idx < pars->size(); idx++) {
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
				lastLine,
				newPosition
			);
			processCursorMove(prev, cursor);
		}
	} else {
		int parIdx = indexOfParagraph(par);
		int lineIdx = par->indexOfLine(line);

		if (lineIdx == 0) {
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
					&((*prevLines)[oldSize]),
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

				// Delete current line.
				lines->remove(lineIdx);

				// Update blocks.
				prevBlock->setParagraph(prevPar);
				block->setParagraph(par);

				// Update cursor to previous line.
				cursor = MarkdownCursor(
					prevBlock,
					lastLine,
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

			// Update block.
			block->setParagraph(par);

			// Update cursor to previous line.
		 	cursor = MarkdownCursor(
				block,
				prevLine,
				newPos
			);
			processCursorMove(prev, cursor);
		}
	}
}

