#include <QWidget>
#include <QScrollArea>
#include <QLine>

#include "widgets/markdown_scroll_widget.h"
#include "widgets/markdown_edit_widget.h"

#include <QDebug>

MarkdownScrollWidget::MarkdownScrollWidget(QWidget *parent)
	: QScrollArea(parent) {}

void MarkdownScrollWidget::setMarkdownWidget(MarkdownEditWidget *w) {
	connect(
		w, &MarkdownEditWidget::cursorMoved,
		this, &MarkdownScrollWidget::onCursorMoved
	);

	setWidget(w);
}

void MarkdownScrollWidget::onCursorMoved(QLine line) {
	ensureVisible(line.x1(), line.y1(), 10, 0);
	ensureVisible(line.x2(), line.y2(), 10, 0);
}

