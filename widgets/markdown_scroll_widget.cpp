#include <QWidget>
#include <QScrollArea>
#include <QLine>
#include <QScrollBar>

#include "widgets/markdown_scroll_widget.h"
#include "widgets/markdown_edit_widget.h"

#include <QDebug>

MarkdownScrollWidget::MarkdownScrollWidget(QWidget *parent)
	: QScrollArea(parent) {}

void MarkdownScrollWidget::setMarkdownWidget(MarkdownEditWidget *w) {
	assert(w != nullptr);

	connect(
		w, &MarkdownEditWidget::cursorMoved,
		this, &MarkdownScrollWidget::onCursorMoved
	);

	m_widget = w;
	setWidget(w);
	m_widget->setPresenter(this);
}

void MarkdownScrollWidget::onCursorMoved(QLine line) {
	ensureVisible(line.x1(), line.y1(), 10, 0);
	ensureVisible(line.x2(), line.y2(), 10, 0);
}

int MarkdownScrollWidget::getPageOffset(bool down) {
	QRect viewportRect = viewport()->rect();
	QScrollBar *bar = verticalScrollBar();

	int oldValue = bar->value();
	int newValue = down
		? std::min(bar->maximum(), bar->value() + bar->pageStep())
		: std::max(0, bar->value() - bar->pageStep());

	// Calculate new position of the viewport.
	int yDiff = float(newValue - oldValue)
		/ (float)(bar->maximum() + bar->pageStep()) 
		* m_widget->rect().height();

	return yDiff;
}

