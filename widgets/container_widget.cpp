#include <QSizePolicy>
#include <QResizeEvent>

#include "widgets/container_widget.h"
#include "widgets/base_widget.h"
#include "widgets/style.h"
#include "widgets/canvas_widget.h"

ContainerWidget::ContainerWidget(
	QWidget *parent,
	Style *style,
	CanvasWidget *canvas
) : BaseWidget(parent, style), m_canvas(canvas) {
	canvas->setParent(this);
}

void ContainerWidget::resizeEvent(QResizeEvent *event) {
	if (m_canvas != nullptr) {
		QSize size = event->size();
		m_canvas->setGeometry(
			0, 0,
			size.width(), size.height()
		);
	}

	BaseWidget::resizeEvent(event);
}

CanvasWidget *ContainerWidget::canvas() {
	return m_canvas;
}

