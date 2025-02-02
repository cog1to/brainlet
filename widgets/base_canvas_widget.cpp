#include <assert.h>
#include <iostream>

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>

#include "layout/base_layout.h"
#include "layout/scroll_area_layout.h"
#include "layout/item_connection.h"
#include "widgets/base_canvas_widget.h"
#include "widgets/scroll_area_widget.h"
#include "widgets/thought_widget.h"

BaseCanvasWidget::BaseCanvasWidget(
	QWidget *parent,
	Style *style,
	BaseLayout *layout
) : BaseWidget(parent, style), m_anchorHighlight(nullptr, style) {
	m_layout = layout;

	setStyleSheet(
		QString("background-color: %1").arg(
			style->background().name(QColor::HexRgb)
		)
	);

	// Update callback.
	layout->onUpdated = [this]{
		this->updateLayout();
		this->update();
	};
}

BaseCanvasWidget::~BaseCanvasWidget() {
	std::unordered_map<ThoughtId, ThoughtWidget*>::iterator wi;
	for (wi = m_widgets.begin(); wi != m_widgets.end(); wi++) {
		delete wi->second;
	}

	std::unordered_map<unsigned int, ScrollAreaWidget*>::iterator sc;
	for (sc = m_scrollAreas.begin(); sc != m_scrollAreas.end(); sc++) {
		delete sc->second;
	}
}

void BaseCanvasWidget::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event);

	if (m_layout != nullptr) {
		m_layout->setSize(event->size());
	}
}

void BaseCanvasWidget::paintEvent(QPaintEvent *) {
	if (m_layout == nullptr) {
		return;
	}

	// Main connections.
	const std::vector<ItemConnection> *connections = m_layout->connections();
	if (connections == nullptr || connections->size() == 0) {
		return;
	}

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QColor color = m_style->activeAnchorColor();

	QPainterPath path;

	for (auto& connection: *connections) {
		auto fromIt = m_widgets.find(connection.from);
		if (fromIt == m_widgets.end() || fromIt->second->parent() == nullptr)
			continue;

		auto toIt = m_widgets.find(connection.to);
		if (toIt == m_widgets.end() || toIt->second->parent() == nullptr)
			continue;

		AnchorPoint outgoing = fromIt->second->getAnchorFrom(connection.type);
		AnchorPoint incoming = toIt->second->getAnchorTo(connection.type);
		qreal dx = std::abs(outgoing.x - incoming.x);
		qreal dy = std::abs(outgoing.y - incoming.y);

		path.moveTo(outgoing.x, outgoing.y);
		path.cubicTo(
			outgoing.x + (outgoing.dx * dx * controlPointRatio), outgoing.y + (outgoing.dy * dy * controlPointRatio),
			incoming.x + (incoming.dx * dx * controlPointRatio), incoming.y + (incoming.dy * dy * controlPointRatio),
			incoming.x, incoming.y
		);
	}

	painter.setPen(QPen(color, 1));
	painter.drawPath(path);

	// These connect nodes placed around the main node with each other.
	const std::vector<ItemConnection> *subconnections = m_layout->subconnections();
	if (subconnections == nullptr || subconnections->size() == 0) {
		return;
	}

	QPainterPath spath;

	for (auto& connection: *subconnections) {
		auto fromIt = m_widgets.find(connection.from);
		if (fromIt == m_widgets.end() || fromIt->second->parent() == nullptr)
			continue;

		auto toIt = m_widgets.find(connection.to);
		if (toIt == m_widgets.end() || toIt->second->parent() == nullptr)
			continue;

		AnchorPoint outgoing = fromIt->second->getAnchorFrom(connection.type);
		AnchorPoint incoming = toIt->second->getAnchorTo(connection.type);
		qreal dx = std::abs(outgoing.x - incoming.x);
		qreal dy = std::abs(outgoing.y - incoming.y);
		qreal cp = controlPointRatio;

		spath.moveTo(outgoing.x, outgoing.y);
		spath.cubicTo(
			outgoing.x + (outgoing.dx * dx * cp), outgoing.y + (outgoing.dy * dy * cp),
			incoming.x + (incoming.dx * dx * cp), incoming.y + (incoming.dy * dy * cp),
			incoming.x, incoming.y
		);
	}

	QColor subColor = color;
	subColor.setAlpha(128);
	painter.setPen(QPen(color, 0.5));
	painter.drawPath(spath);

	if (m_anchorHighlight.parent() != nullptr && m_anchorSource != nullptr)
		drawAnchorConnection(painter);
}

void BaseCanvasWidget::drawAnchorConnection(QPainter& painter) {
	assert(m_anchorHighlight.parent() !=  nullptr);
	assert(m_anchorSource !=  nullptr);

	QRect highlightCenter = m_anchorHighlight.geometry();

	// Incoming point is the center of the activated anchor of the ThoughtWidget.
	AnchorPoint incoming = m_anchorSource->widget->getAnchorFrom(
		m_anchorSource->type
	);

	// Outgoing point is the center of the higlight view, with control point
	// mirrored.
	AnchorPoint outgoing = AnchorPoint{
		.x = highlightCenter.x() + highlightCenter.width() / 2.0,
		.y = highlightCenter.y() + highlightCenter.height() / 2.0,
		.dx = -incoming.dx,
		.dy = -incoming.dy,
	};

	QPainterPath path;

	// Add highlight path.
	path.moveTo(outgoing.x, outgoing.y);
	qreal dx = std::abs(outgoing.x - incoming.x);
	qreal dy = std::abs(outgoing.y - incoming.y);
	qreal cp = controlPointRatio;

	// TODO: Revise this math. Looks kinda ok, but constants/ratios may be
	// modified for better look & feel.
	qreal mdx = std::max(dx, 50.0*std::min(dx/10.0, 1.0));
	qreal mdy = std::max(dy, 50.0*std::min(dy/10.0, 1.0));

	path.moveTo(outgoing.x, outgoing.y);
	path.cubicTo(
		outgoing.x + (outgoing.dx * mdx * cp),
		outgoing.y + (outgoing.dy * mdy * cp),
		incoming.x + (incoming.dx * mdx * cp),
		incoming.y + (incoming.dy * mdy * cp),
		incoming.x,
		incoming.y
	);

	painter.setPen(QPen(m_style->anchorHighlight(), 1, Qt::DashLine));
	painter.drawPath(path);
}

void BaseCanvasWidget::updateLayout() {
	if (m_layout == nullptr)
		return;

	const ThoughtId *main = m_layout->rootId();
	if (main == nullptr)
		return;

	QPoint cursor = mapFromGlobal(QCursor::pos());
	const std::unordered_map<ThoughtId, ItemLayout> *items = m_layout->items();

	// Layout scroll areas first.
	layoutScrollAreas();

	// Layout all widgets.
	std::unordered_map<ThoughtId, ItemLayout>::const_iterator it;
	for (it = items->begin(); it != items->end(); it++) {
		ThoughtWidget *widget = cachedWidget(it->first);
		if (widget == nullptr) {
			widget = createWidget(it->second, it->first != *main);
		} else {
			if (widget->text() != *it->second.name)
				widget->setText(*it->second.name);
		}

		// Save the widget to the cache.
		m_widgets.insert_or_assign(it->first, widget);

		// Place the widget in the canvas.
		const ItemLayout& layout = it->second;
		widget->setGeometry(
			layout.x, layout.y,
			layout.w, layout.h
		);
		if (widget->isActive() || (
			(cursor.x() >= layout.x) && (cursor.x() <= layout.x + layout.w) &&
			(cursor.y() >= layout.y) && (cursor.y() <= layout.y + layout.h)
		)) {
			onWidgetActivated(widget);
		}

		// We raise the widget to prevent it being obstructed by scroll areas and
		// any decorator/background view.
		widget->raise();

		// Reset the parent in case the widget is a newly created one.
		widget->setParent(this);

		// Trigger visiblility/repaint in case the wieget is a newly created one.
		widget->show();
	}

	// Remove unused widgets.
	std::unordered_map<ThoughtId, ThoughtWidget*>::iterator wit;
	for (wit = m_widgets.begin(); wit != m_widgets.end(); wit++) {
		if (wit->first == *main) {
			continue;
		}

		if (items->find(wit->first) != items->end()) {
			continue;
		}

		wit->second->setParent(nullptr);
	}
}

// Scroll areas.

void BaseCanvasWidget::layoutScrollAreas() {
	const std::unordered_map<unsigned int, ScrollAreaLayout> *scrolls = m_layout->scrollAreas();
	std::unordered_map<unsigned int, ScrollAreaLayout>::const_iterator it;

	for (it = scrolls->begin(); it != scrolls->end(); it++) {
		ScrollAreaWidget *widget = cachedScrollArea(it->first);
		const ScrollAreaLayout &layout = it->second;

		if (widget == nullptr) {
			widget = createScrollArea(it->first, layout.pos);
			m_scrollAreas.insert({it->first, widget});
		}

		widget->setScrollBarPos(layout.pos);
		widget->setScrollBarSettings(layout.barWidth, layout.offset);

		widget->setGeometry(
			layout.x, layout.y,
			layout.w, layout.h
		);

		widget->setParent(this);
		widget->show();
	}

	// Remove unused widgets
	std::unordered_map<unsigned int, ScrollAreaWidget*>::iterator wit;
	for (wit = m_scrollAreas.begin(); wit != m_scrollAreas.end(); wit++) {
		if (scrolls->find(wit->first) != scrolls->end()) {
			continue;
		}

		wit->second->setParent(nullptr);
	}
}

// Caching widgets.

ScrollAreaWidget *BaseCanvasWidget::cachedScrollArea(unsigned int id) {
	if (auto search = m_scrollAreas.find(id); search != m_scrollAreas.end()) {
		return search->second;
	}
	return nullptr;
}

ScrollAreaWidget *BaseCanvasWidget::createScrollArea(
	unsigned int id,
	ScrollBarPos pos
) {
	ScrollAreaWidget *widget = new ScrollAreaWidget(this,	m_style, id, pos);

	QObject::connect(
		widget, SIGNAL(scrolled(unsigned int, int)),
		this, SLOT(onScrollAreaScroll(unsigned int, int))
	);

	return widget;
}

ThoughtWidget *BaseCanvasWidget::cachedWidget(ThoughtId id) {
	if (auto search = m_widgets.find(id); search != m_widgets.end()) {
		return search->second;
	}
	return nullptr;
}

ThoughtWidget *BaseCanvasWidget::createWidget(
	const ItemLayout& layout,
	bool readonly
) {
	ThoughtWidget *widget = new ThoughtWidget(
		this,
		m_style,
		layout.id,
		readonly,
		*layout.name,
		layout.hasParents, layout.hasChildren, layout.hasLinks
	);

	QObject::connect(
		widget, SIGNAL(activated(ThoughtWidget*)),
		this, SLOT(onWidgetActivated(ThoughtWidget*))
	);

	QObject::connect(
		widget, SIGNAL(deactivated(ThoughtWidget*)),
		this, SLOT(onWidgetDeactivated(ThoughtWidget*))
	);

	QObject::connect(
		widget, SIGNAL(textChanged(ThoughtWidget*)),
		this, SLOT(onWidgetActivated(ThoughtWidget*))
	);

	QObject::connect(
		widget, SIGNAL(mouseScroll(ThoughtWidget*, QWheelEvent*)),
		this, SLOT(onWidgetScroll(ThoughtWidget*, QWheelEvent*))
	);

	QObject::connect(
		widget, SIGNAL(anchorEntered(ThoughtWidget*, AnchorType, QPoint)),
		this, SLOT(onAnchorEntered(ThoughtWidget*, AnchorType, QPoint))
	);

	QObject::connect(
		widget, SIGNAL(anchorLeft()),
		this, SLOT(onAnchorLeft())
	);

	QObject::connect(
		widget, SIGNAL(anchorMoved(QPoint)),
		this, SLOT(onAnchorMoved(QPoint))
	);

	QObject::connect(
		widget, SIGNAL(textConfirmed(ThoughtWidget*, QString, std::function<void(bool)>)),
		this, SLOT(onTextConfirmed(ThoughtWidget*, QString, std::function<void(bool)>))
	);

	return widget;
}

// Slots

void BaseCanvasWidget::onWidgetActivated(ThoughtWidget* widget) {
	QPoint wpos = widget->pos();
	QSize wsize = widget->size();
	QSize center = QSize(
		wpos.x() + wsize.width() / 2,
		wpos.y() + wsize.height() / 2
	);

	QSize fullSize = widget->sizeForWidth((float)size().width() * 0.5);

	int x = center.width() - fullSize.width() / 2;
	int y = center.height() - fullSize.height() / 2;

	// If we're out of bounds, move inbounds.
	if (x < 0) {
		x = 10;
	} else if (x + fullSize.width() > size().width()) {
		x = size().width() - 10 - fullSize.width();
	}

	if (y < 0) {
		y = 10;
	} else if (y + fullSize.height() > size().height()) {
		y = size().height() - 10 - fullSize.height();
	}

	// Bring widget to the top of the stack so it will paint over others.
	widget->raise();

	// Expand.
	widget->setGeometry(
		x, y,
		fullSize.width(), fullSize.height()
	);

	// Repaint to update connections.
	update();
}

void BaseCanvasWidget::onWidgetDeactivated(ThoughtWidget* widget) {
	if (m_layout == nullptr)
		return;

	const std::unordered_map<ThoughtId, ItemLayout> *items = m_layout->items();
	if (auto search = items->find(widget->id()); search != items->end()) {
		ItemLayout layout = search->second;
		widget->setGeometry(
			layout.x, layout.y,
			layout.w, layout.h
		);
	}

	// Repaint to update connections.
	update();
}

void BaseCanvasWidget::onWidgetScroll(ThoughtWidget* widget, QWheelEvent* event) {
	QPointF position = event->position();
	QPointF globalPos = widget->mapToParent(position);

	std::unordered_map<unsigned int, ScrollAreaWidget*>::iterator it;
	for (it = m_scrollAreas.begin(); it != m_scrollAreas.end(); it++) {
		QRect rect = it->second->frameGeometry();
		if (
			globalPos.x() >= rect.x() &&
			globalPos.y() >= rect.y() &&
			globalPos.x() <= rect.x() + rect.width() &&
			globalPos.y() <= rect.y() + rect.height()
		) {
			it->second->wheelEvent(event);
		}
	}
}

void BaseCanvasWidget::onScrollAreaScroll(unsigned int id, int value) {
	if (m_layout == nullptr)
		return;

	// Reload layout to update visible widgets.
	m_layout->onScroll(id, value);
}

void BaseCanvasWidget::onAnchorEntered(
	ThoughtWidget* widget,
	AnchorType type,
	QPoint center
) {
	const QSize highlightSize(20, 20);

	m_anchorHighlight.setGeometry(
		center.x() - highlightSize.width() / 2,
		center.y() - highlightSize.height() / 2,
		highlightSize.width(),
		highlightSize.height()
	);

	m_anchorHighlight.setParent(this);
	m_anchorHighlight.show();

	// Save the source for drawing connections.
	m_anchorSource = new AnchorSource(widget, type);
}

void BaseCanvasWidget::onAnchorLeft() {
	m_anchorHighlight.setParent(nullptr);

	if (m_anchorSource != nullptr) {
		delete m_anchorSource;
		m_anchorSource = nullptr;
	}

	// Redraw.
	update();
}

void BaseCanvasWidget::onAnchorMoved(QPoint point) {
	const QSize highlightSize(20, 20);

	m_anchorHighlight.setGeometry(
		point.x() - highlightSize.width() / 2,
		point.y() - highlightSize.height() / 2,
		highlightSize.width(),
		highlightSize.height()
	);

	// Redraw.
	update();
}

void BaseCanvasWidget::onTextConfirmed(
	ThoughtWidget *source,
	QString text,
	std::function<void(bool)> callback
) {
	ThoughtId id = source->id();
	emit textChanged(id, text, callback);
}

