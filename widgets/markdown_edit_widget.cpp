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
		(key == Qt::Key_Left && event->modifiers() & Qt::ControlModifier)
	) {
		cursor = documentStart();
		processCursorMove(prev, cursor);
	} else if (
		key == Qt::Key_End ||
		(key == Qt::Key_Right && event->modifiers() & Qt::ControlModifier)
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
				cursor.position = prevLine->text.length();
				processCursorMove(prev, cursor);
			} else if (
				MarkdownBlock *prevBlock = blockBefore(block);
				prevBlock != nullptr
			) {
				QList<text::Line> *prevLines = prevBlock->paragraph()->getLines();
				cursor.block = prevBlock;
				cursor.line = &((*prevLines)[prevLines->size() - 1]);
				cursor.position = cursor.line->text.length();
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
		QList<text::FormatRange> *ranges = &to.line->formats;

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

