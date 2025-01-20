#include <QWheelEvent>
#include <iostream>

#include "widgets/scroll_area_widget.h"
#include "layout/scroll_area_layout.h"

ScrollAreaWidget::ScrollAreaWidget(QWidget* parent, Style* style, ScrollBarPos pos)
	: BaseWidget(parent, style)
{
	m_pos = pos;
}

void ScrollAreaWidget::setScrollBarPos(ScrollBarPos pos) {
	m_pos = pos;
	update();
}

void ScrollAreaWidget::wheelEvent(QWheelEvent *event) {
	std::cout << "wheel: " << (event->angleDelta().y() / 8) << "\n";
}
