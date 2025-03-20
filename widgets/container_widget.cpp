#include <QSizePolicy>
#include <QResizeEvent>

#include "widgets/container_widget.h"
#include "widgets/base_widget.h"
#include "widgets/style.h"
#include "widgets/canvas_widget.h"

#include <QDebug>

ContainerWidget::ContainerWidget(
	QWidget *parent,
	Style *style,
	CanvasWidget *canvas
) : BaseWidget(parent, style),
	m_canvas(canvas),
	m_search(this, style, false)
{
	canvas->setParent(this);
	m_search.raise();

	// Events.
	connect(
		&m_search, SIGNAL(searchCanceled(SearchWidget*)),
		this, SLOT(onSearchCanceled(SearchWidget*))
	);
	connect(
		&m_search, SIGNAL(searchActivated(SearchWidget*)),
		this, SLOT(onSearchActivated(SearchWidget*))
	);
}

void ContainerWidget::resizeEvent(QResizeEvent *event) {
	// Stretch canvas to full size.
	if (m_canvas != nullptr) {
		QSize size = event->size();
		m_canvas->setGeometry(
			0, 0,
			size.width(), size.height()
		);
	}

	layoutSearch();

	BaseWidget::resizeEvent(event);
}

CanvasWidget *ContainerWidget::canvas() {
	return m_canvas;
}

// Slots.

void ContainerWidget::onSearchCanceled(SearchWidget *widget) {
	widget->clear();
	layoutSearch();
}

void ContainerWidget::onSearchActivated(SearchWidget *widget) {
	layoutSearch();
}

// Helpers

void ContainerWidget::layoutSearch() {
	QSize current = size();
	QSize hint = m_search.sizeHint();
	m_search.setGeometry(
		0, 0,
		m_search.isActive() ? (current.width() * 2 / 3) : 92,
		hint.height()
	);
}
