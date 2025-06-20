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
) : BaseWidget(parent, style),
	m_canvas(canvas),
	m_search(this, style, false, tr("Go to..."), false),
	m_history(this, style)
{
	canvas->setParent(this);
	m_search.raise();
	m_history.raise();

	// Events.
	connect(
		&m_search, SIGNAL(searchCanceled(SearchWidget*)),
		this, SLOT(onSearchCanceled(SearchWidget*))
	);
	connect(
		&m_search, SIGNAL(searchFocusLost(SearchWidget*)),
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
	QSize historyHint = m_history.sizeHint();

	// Stretch canvas to full size.
	if (m_canvas != nullptr) {
		QSize size = event->size();
		m_canvas->setGeometry(
			0, 0,
			size.width(), size.height() - historyHint.height() - 4
		);
	}

	updateSearchWidth();
	layoutSearch();
	layoutHistory();
}

CanvasWidget *ContainerWidget::canvas() {
	return m_canvas;
}

SearchWidget *ContainerWidget::search() {
	return &m_search;
}

HistoryWidget *ContainerWidget::history() {
	return &m_history;
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

void ContainerWidget::layoutHistory() {
	QSize current = size();
	QSize hint = m_history.sizeHint();

	m_history.setGeometry(
		0, current.height() - hint.height(),
		current.width(), hint.height()
	);
}

