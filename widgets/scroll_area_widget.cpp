#include <QWheelEvent>
#include <QResizeEvent>
#include <iostream>

#include "widgets/scroll_bar_widget.h"
#include "widgets/scroll_area_widget.h"
#include "layout/scroll_area_layout.h"

ScrollAreaWidget::ScrollAreaWidget(
	QWidget* parent,
	Style* style,
	unsigned int id,
	ScrollBarPos pos
)
	: BaseWidget(parent, style), m_bar(this, style, pos)
{
	m_id = id;
	m_pos = pos;
}

void ScrollAreaWidget::setScrollBarPos(ScrollBarPos pos) {
	m_pos = pos;
	update();
}

void ScrollAreaWidget::setScrollBarSettings(float width, float offset) {
	m_bar.setSettings(width, offset);
}

void ScrollAreaWidget::wheelEvent(QWheelEvent *event) {
	emit scrolled(m_id, ((event->angleDelta().y() / 8) > 0) ? -1 : 1);
}

void ScrollAreaWidget::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event);
	const int width = m_style->scrollWidth();

	// Layout bar.
	switch (m_pos) {
	case ScrollBarPos::Left:
		m_bar.setGeometry(
			0, 0,
			width, event->size().height()
		);
		break;
	case ScrollBarPos::Right:
		m_bar.setGeometry(
			event->size().width() - width, 0,
			width, event->size().height()
		);
		break;
	case ScrollBarPos::Top:
		m_bar.setGeometry(
			0, 0,
			event->size().width(), width
		);
		break;
	case ScrollBarPos::Bottom:
		m_bar.setGeometry(
			0, event->size().height() - width,
			event->size().width(), width
		);
		break;
	}
}

