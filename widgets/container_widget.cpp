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
	connect(
		&m_search, SIGNAL(updated(SearchWidget*)),
		this, SLOT(onSearchUpdated(SearchWidget*))
	);
}

void ContainerWidget::resizeEvent(QResizeEvent *event) {
	BaseWidget::resizeEvent(event);

	// Stretch canvas to full size.
	if (m_canvas != nullptr) {
		QSize size = event->size();
		m_canvas->setGeometry(
			0, 0,
			size.width(), size.height()
		);
	}

	updateSearchWidth();
	layoutSearch();
}

CanvasWidget *ContainerWidget::canvas() {
	return m_canvas;
}

SearchWidget *ContainerWidget::search() {
	return &m_search;
}

// Slots.

void ContainerWidget::onSearchCanceled(SearchWidget *widget) {
	widget->clear();
	updateSearchWidth();
	layoutSearch();
}

void ContainerWidget::onSearchActivated(SearchWidget *widget) {
	updateSearchWidth();
	layoutSearch();
}

void ContainerWidget::onSearchUpdated(SearchWidget *widget) {
	updateSearchWidth();
	layoutSearch();
}

// Helpers

void ContainerWidget::updateSearchWidth() {
	int width = m_search.isActive() ? (size().width() / 2) : 94;
	m_search.setMaximumWidth(width);
	m_search.setMinimumWidth(width);
}

void ContainerWidget::layoutSearch() {
	QSize current = size();
	QSize hint = m_search.sizeHint();
	m_search.setGeometry(
		0, 0,
		m_search.isActive() ? (current.width() / 2) : 94,
		hint.height()
	);
}

