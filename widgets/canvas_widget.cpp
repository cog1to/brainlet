#include <assert.h>
#include <iostream>

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include <QShowEvent>
#include <QMenu>
#include <QTranslator>

#include "layout/base_layout.h"
#include "layout/scroll_area_layout.h"
#include "layout/item_connection.h"
#include "widgets/canvas_widget.h"
#include "widgets/scroll_area_widget.h"
#include "widgets/thought_widget.h"

CanvasWidget::CanvasWidget(
	QWidget *parent,
	Style *style,
	BaseLayout *layout
) : BaseWidget(parent, style),
	m_anchorHighlight(this, style),
	m_overlay(this)
{
	m_layout = layout;
	m_anchorHighlight.hide();
	m_overlay.setStyleSheet("background-color: #00000000");

	// Enable mouse tracking for highlighting connections.
	setMouseTracking(true);

	setStyleSheet(
		QString("background-color: %1").arg(
			style->background().name(QColor::HexRgb)
		)
	);

	// Size policy.
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	// Update callback.
	layout->onUpdated = [this]{
		this->updateLayout();
		this->update();
	};
}

CanvasWidget::~CanvasWidget() {
	std::unordered_map<ThoughtId, ThoughtWidget*>::iterator wi;
	for (wi = m_widgets.begin(); wi != m_widgets.end(); wi++) {
		delete wi->second;
	}

	std::unordered_map<unsigned int, ScrollAreaWidget*>::iterator sc;
	for (sc = m_scrollAreas.begin(); sc != m_scrollAreas.end(); sc++) {
		delete sc->second;
	}
}

QSize CanvasWidget::sizeHint() const {
	return QSize(100, 100);
}

void CanvasWidget::showSuggestions(std::vector<ConnectionItem> items) {
	if (m_newThought == nullptr)
		return;

	if (items.size() == 0) {
		hideSuggestions();
		return;
	}

	if (m_suggestions == nullptr) {
		m_suggestionsContainer = new QFrame(this);
		m_suggestionsContainer->setStyleSheet(
			QString("background: #64000000; border-radius: 10px;")
		);

		m_suggestions = new ConnectionListWidget(
			m_suggestionsContainer,
			m_style,
			false
		);

	connect(
		m_suggestions, SIGNAL(thoughtSelected(ThoughtId, QString)),
		this, SLOT(onSuggestionSelected(ThoughtId, QString))
	);
	}

	m_suggestions->setItems(items);
	layoutSuggestions();
}

void CanvasWidget::hideSuggestions() {
	if (m_suggestions != nullptr) {
		delete m_suggestions;
		m_suggestions = nullptr;
	}

	if (m_suggestionsContainer != nullptr) {
		delete m_suggestionsContainer;
		m_suggestionsContainer = nullptr;
	}

	update();
}

void CanvasWidget::showError(QString error) {
	if (m_error != nullptr)
		delete m_error;

	m_error = new ToastWidget(m_style, error);
	m_error->show(this);
}

// Events

void CanvasWidget::showEvent(QShowEvent *) {
	emit shown();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
	if (m_anchorSource != nullptr || m_menuThought != nullptr) {
		return;
	}

	QPoint pos = event->pos();

	// Rectangle around cursor to capture connections.
	QRectF activeRect = QRectF(
		pos.x() - 10,
		pos.y() - 10,
		20,
		20
	);

	std::vector<Path> intersects;
	for (auto& p: m_paths) {
		// Not very smart. We just sample the curve along the path and check if
		// some point gets inside the capture rectangle. But it still might be
		// better than trying to solve a polinomial equation of fifth order...
		//
		// As a potential optimization, pre-sample the curve at the point of
		// construction, i. e. in makePath() method.
		for (int i = 0; i <= 25; i++) {
			qreal percent = (qreal)i / 25.0;
			QPointF point = p.path.pointAtPercent(percent);
			if (activeRect.contains(point)) {
				intersects.push_back(p);
			}
		}
	}

	if (intersects.size() == 0 && m_pathHighlight.has_value()) {
		m_pathHighlight = std::nullopt;
		update();
	} else if (
		intersects.size() > 0 &&
		(
			!m_pathHighlight.has_value() ||
			m_pathHighlight.value().path != intersects[0].path)
		)
	{
		// TODO: take closest.
		QPen pen(m_style->anchorHighlight(), 1, Qt::DashLine);
		m_pathHighlight = Path(intersects[0].from, intersects[0].to, pen, intersects[0].path);
		update();
	}
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
	if (event->button() != Qt::RightButton || !m_pathHighlight.has_value()) {
		return;
	}

	QPoint point = event->pos();

	QMenu contextMenu(this);
	if (m_style != nullptr) {
		contextMenu.setStyleSheet(m_style->menuStyle());
	}

	QString disconnectMenu = tr("Disconnect");
	QAction action(disconnectMenu, this);
	connect(&action, SIGNAL(triggered()), this, SLOT(onDisconnect()));

	contextMenu.addAction(&action);
	contextMenu.exec(mapToGlobal(point));
}

void CanvasWidget::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event);

	if (m_layout != nullptr) {
		m_layout->setSize(event->size());
	}
}

void CanvasWidget::paintEvent(QPaintEvent *event) {
	if (m_layout == nullptr) {
		return;
	}

	// Main connections.

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	for (auto& path: m_paths) {
		drawConnection(painter, path);
	}

	// Editing.
	if (m_anchorSource != nullptr) {
		if (m_overThought != nullptr) {
			drawOverThoughtConnection(painter);
		} else if (m_anchorHighlight.isVisible()) {
			drawAnchorConnection(painter);
		} else if (m_newThought != nullptr && m_newThought->isVisible()) {
			drawNewThoughtConnection(painter);
		}
	}

	// Highlight.
	if (m_pathHighlight.has_value()) {
		drawConnection(painter, m_pathHighlight.value());
	}
}

// Connections helpers.

void CanvasWidget::drawAnchorConnection(QPainter& painter) {
	assert(m_anchorHighlight.isVisible());
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

	QPen pen(m_style->anchorHighlight(), 1, Qt::DashLine);
	Path path = makePath(m_anchorSource->widget, nullptr, outgoing, incoming, pen);

	drawConnection(
		painter,
		path
	);
}

void CanvasWidget::drawNewThoughtConnection(QPainter& painter) {
	assert(m_newThought != nullptr && m_newThought->isVisible());
	assert(m_anchorSource !=  nullptr);

	// Incoming point is the widget's anchor.
	AnchorType incomingType = reverseAnchorType(m_anchorSource->type);
	AnchorPoint incoming = m_newThought->getAnchorFrom(
		incomingType
	);

	// Outgoing point is from the source widget.
	AnchorPoint outgoing = m_anchorSource->widget->getAnchorFrom(
		m_anchorSource->type
	);

	QPen pen(m_style->anchorHighlight(), 1, Qt::DashLine);
	Path path = makePath(m_anchorSource->widget, m_newThought, outgoing, incoming, pen);

	drawConnection(
		painter,
		path
	);
}

void CanvasWidget::drawOverThoughtConnection(QPainter& painter) {
	assert(m_overThought != nullptr);
	assert(m_anchorSource !=  nullptr);

	// Incoming point is the center of the activated anchor of the ThoughtWidget.
	AnchorType incomingType = reverseAnchorType(m_anchorSource->type);
	AnchorPoint incoming = m_overThought->getAnchorFrom(
		incomingType
	);

	// Outgoing point is from the source widget.
	AnchorPoint outgoing = m_anchorSource->widget->getAnchorFrom(
		m_anchorSource->type
	);

	QPen pen(m_style->anchorHighlight(), 1, Qt::DashLine);
	Path path = makePath(m_anchorSource->widget, m_overThought, outgoing, incoming, pen);

	drawConnection(
		painter,
		path
	);
}

Path CanvasWidget::makePath(
	ThoughtWidget *from, ThoughtWidget *to,
	AnchorPoint outgoing, AnchorPoint incoming,
	QPen& pen
) {
	QPainterPath path;

	// TODO: Revise this math. Looks kinda ok, but constants/ratios may be
	// modified for better look & feel.

	// Add highlight path.
	path.moveTo(outgoing.x, outgoing.y);
	qreal dx = std::abs(outgoing.x - incoming.x);
	qreal dy = std::abs(outgoing.y - incoming.y);
	qreal cp = controlPointRatio;

	qreal mdx = std::max(dx, 50.0*std::min(dx/10.0, 1.0));
	qreal mdy = std::max(dy, 50.0*std::min(dy/10.0, 1.0));

	qreal cp1x = (outgoing.dx * mdx * cp);
	qreal cp1y = (outgoing.dy * mdy * cp);
	qreal cp2x = (incoming.dx * mdx * cp);
	qreal cp2y = (incoming.dy * mdy * cp);

	// Special case for connections that are aligned on lines parallel to axes.
	// It avoids the connection going straight through one of the widgets.
	// TODO: This is really ugly. Need to think about the underlying math.
	if (
		dx > minAnchorDistance &&
		dy < minAnchorDistance &&
		incoming.dx != 0 && incoming.dx == outgoing.dx
	) {
		cp1y = -minAnchorDistance;
		cp2y = -minAnchorDistance;
	} else if (
		dx > minAnchorDistance &&
		dy < minAnchorDistance &&
		incoming.dy != 0 && incoming.dy == -outgoing.dy
	) {
		cp1y = outgoing.dy*minAnchorDistance;
		cp2y = incoming.dy*minAnchorDistance;
	} else if (
		dy > minAnchorDistance &&
		dx < minAnchorDistance &&
		incoming.dx != 0 && incoming.dx == outgoing.dx
	) {
		cp1x = outgoing.dx*minAnchorDistance;
		cp2x = incoming.dx*minAnchorDistance;
	}

	path.moveTo(outgoing.x, outgoing.y);
	path.cubicTo(
		outgoing.x + cp1x,
		outgoing.y + cp1y,
		incoming.x + cp2x,
		incoming.y + cp2y,
		incoming.x,
		incoming.y
	);

	return Path(from, to, pen, path);
}

inline void CanvasWidget::drawConnection(
	QPainter& painter,
	Path& path
) {
	painter.setPen(path.pen);
	painter.drawPath(path.path);
}

void CanvasWidget::updateLayout() {
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
			// Important: remove focus BEFORE setting read-only state,
			// otherwise widget will retain "hidden" focus and will get
			// automatically reactivated when selected again.
			if (it->first != *main && widget->isActive())
				widget->removeFocus();

			widget->setReadOnly(it->first != *main);

			if (widget->text() != it->second.name)
				widget->setText(it->second.name);
		}

		// Save the widget to the cache.
		m_widgets.insert_or_assign(it->first, widget);

		const ItemLayout& layout = it->second;

		// Set widget's link orientation and connection markers.
		widget->setRightSideLink(layout.rightSideLink);
		widget->setHasParent(layout.hasParents);
		widget->setHasChild(layout.hasChildren);
		widget->setHasLink(layout.hasLinks);
		widget->setCanDelete(layout.canDelete);
		widget->setFocused(it->first == *main);

		// Place the widget in the canvas.
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

		// Hide.
		wit->second->setParent(nullptr);
	}

	// Recalculate paths.
	updatePaths();
}

void CanvasWidget::updatePaths() {
	if (m_layout == nullptr) {
		return;
	}

	// Main connections.
	const std::vector<ItemConnection> *connections = m_layout->connections();
	if (connections == nullptr || connections->size() == 0) {
		return;
	}

	// Clear old paths.
	m_paths.clear();

	QColor color = m_style->activeAnchorColor();
	QPen pen = QPen(color, 1);

	for (auto& connection: *connections) {
		auto fromIt = m_widgets.find(connection.from);
		if (fromIt == m_widgets.end() || fromIt->second->parent() == nullptr)
			continue;

		auto toIt = m_widgets.find(connection.to);
		if (toIt == m_widgets.end() || toIt->second->parent() == nullptr)
			continue;

		AnchorPoint outgoing = fromIt->second->getAnchorFrom(connection.type);
		AnchorPoint incoming = toIt->second->getAnchorTo(connection.type);

		Path path = makePath(fromIt->second, toIt->second, outgoing, incoming, pen);
		m_paths.push_back(path);
	}

	// These connect nodes placed around the main node with each other.
	const std::vector<ItemConnection> *subconnections = m_layout->subconnections();
	if (subconnections == nullptr || subconnections->size() == 0) {
		return;
	}

	QPen subPen = QPen(color, 0.5);
	subPen.setStyle(Qt::DashLine);
	subPen.setDashPattern({4, 2});

	for (auto& connection: *subconnections) {
		auto fromIt = m_widgets.find(connection.from);
		if (fromIt == m_widgets.end() || fromIt->second->parent() == nullptr)
			continue;

		auto toIt = m_widgets.find(connection.to);
		if (toIt == m_widgets.end() || toIt->second->parent() == nullptr)
			continue;

		AnchorPoint outgoing = fromIt->second->getAnchorFrom(connection.type);
		AnchorPoint incoming = toIt->second->getAnchorTo(connection.type);

		Path path = makePath(fromIt->second, toIt->second, outgoing, incoming, subPen);
		m_paths.push_back(path);
	}
}

// Scroll areas.

void CanvasWidget::layoutScrollAreas() {
	const std::unordered_map<unsigned int, ScrollAreaLayout> *scrolls =
		m_layout->scrollAreas();
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

ScrollAreaWidget *CanvasWidget::cachedScrollArea(unsigned int id) {
	if (auto search = m_scrollAreas.find(id); search != m_scrollAreas.end()) {
		return search->second;
	}
	return nullptr;
}

ScrollAreaWidget *CanvasWidget::createScrollArea(
	unsigned int id,
	ScrollBarPos pos
) {
	ScrollAreaWidget *widget = new ScrollAreaWidget(this,	m_style, id, pos);

	connect(
		widget, SIGNAL(scrolled(unsigned int, int)),
		this, SLOT(onScrollAreaScroll(unsigned int, int))
	);

	return widget;
}

ThoughtWidget *CanvasWidget::cachedWidget(ThoughtId id) {
	if (auto search = m_widgets.find(id); search != m_widgets.end()) {
		return search->second;
	}
	return nullptr;
}

ThoughtWidget *CanvasWidget::createWidget(
	const ItemLayout& layout,
	bool readonly
) {
	ThoughtWidget *widget = new ThoughtWidget(
		this,
		m_style,
		layout.id,
		readonly,
		layout.name,
		layout.hasParents, layout.hasChildren, layout.hasLinks
	);

	connectWidget(widget);

	return widget;
}

void CanvasWidget::connectWidget(ThoughtWidget *widget) {
	connect(
		widget, SIGNAL(clicked(ThoughtWidget*)),
		this, SLOT(onWidgetClicked(ThoughtWidget*))
	);
	connect(
		widget, SIGNAL(activated(ThoughtWidget*)),
		this, SLOT(onWidgetActivated(ThoughtWidget*))
	);
	connect(
		widget, SIGNAL(deactivated(ThoughtWidget*)),
		this, SLOT(onWidgetDeactivated(ThoughtWidget*))
	);
	connect(
		widget, SIGNAL(textChanged(ThoughtWidget*)),
		this, SLOT(onWidgetActivated(ThoughtWidget*))
	);
	connect(
		widget, SIGNAL(mouseScroll(ThoughtWidget*, QWheelEvent*)),
		this, SLOT(onWidgetScroll(ThoughtWidget*, QWheelEvent*))
	);
	connect(
		widget, SIGNAL(anchorEntered(ThoughtWidget*, AnchorType, QPoint)),
		this, SLOT(onAnchorEntered(ThoughtWidget*, AnchorType, QPoint))
	);
	connect(
		widget, SIGNAL(anchorLeft()),
		this, SLOT(onAnchorLeft())
	);
	connect(
		widget, SIGNAL(anchorMoved(QPoint)),
		this, SLOT(onAnchorMoved(QPoint))
	);
	connect(
		widget, SIGNAL(anchorReleased(QPoint)),
		this, SLOT(onAnchorReleased(QPoint))
	);
	connect(
		widget, SIGNAL(anchorCanceled()),
		this, SLOT(onAnchorCanceled())
	);
	connect(
		widget,
		SIGNAL(textConfirmed(ThoughtWidget*, QString, std::function<void(bool)>)),
		this,
		SLOT(onTextConfirmed(ThoughtWidget*, QString, std::function<void(bool)>))
	);
	connect(
		widget, SIGNAL(menuRequested(ThoughtWidget*, const QPoint&)),
		this, SLOT(onMenuRequested(ThoughtWidget*, const QPoint&))
	);
}

// Slots

void CanvasWidget::onWidgetClicked(ThoughtWidget* widget) {
	emit thoughtSelected(widget->id());
}

void CanvasWidget::onWidgetActivated(ThoughtWidget* widget) {
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
	updatePaths();
	update();
}

void CanvasWidget::onWidgetDeactivated(ThoughtWidget* widget) {
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
	updatePaths();
	update();
}

void CanvasWidget::onWidgetScroll(
	ThoughtWidget* widget,
	QWheelEvent* event
) {
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

void CanvasWidget::onScrollAreaScroll(unsigned int id, int value) {
	if (m_layout == nullptr)
		return;

	// Reload layout to update visible widgets.
	m_layout->onScroll(id, value);
}

void CanvasWidget::onAnchorEntered(
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

	if (m_newThought == nullptr) {
		m_newThought = new ThoughtWidget(
			this, m_style, InvalidThoughtId, false, "",
			false, false, false,
			false, false, false
		);

		// Initial setup.
		m_newThought->setAttribute(Qt::WA_TransparentForMouseEvents, true);
		m_newThought->setHasParent(AnchorType::AnchorChild == type);
		m_newThought->setHasLink(AnchorType::AnchorLink == type);
		m_newThought->setHasChild(AnchorType::AnchorParent == type);
		m_newThought->setRightSideLink(
			type == AnchorType::AnchorLink && widget->id() == *m_layout->rootId()
		);

		// Connect signals.
		connect(
			m_newThought, SIGNAL(textCanceled(ThoughtWidget*)),
			this, SLOT(onCreateCanceled(ThoughtWidget*))
		);
		connect(
			m_newThought,
			SIGNAL(textConfirmed(ThoughtWidget*, QString, std::function<void(bool)>)),
			this,
			SLOT(onCreateConfirmed(ThoughtWidget*, QString, std::function<void(bool)>))
		);
		connect(
			m_newThought, SIGNAL(textChanged(ThoughtWidget*)),
			this, SLOT(onNewThoughtTextChanged(ThoughtWidget*))
		);
		connect(
			m_newThought, SIGNAL(nextSuggestion()),
			this, SLOT(onNextSuggestion())
		);
		connect(
			m_newThought, SIGNAL(prevSuggestion()),
			this, SLOT(onPrevSuggestion())
		);

		// Draw over everything else.
		m_newThought->raise();
	}

	m_anchorHighlight.raise();
	m_anchorHighlight.show();

	// Save the source for drawing connections.
	m_anchorSource = new AnchorSource(widget, type);

	// Deselect higlighted path.
	if (m_pathHighlight.has_value()) {
		m_pathHighlight = std::nullopt;
		update();
	}
}

void CanvasWidget::onAnchorLeft() {
	// Only clear anchors if we're not creating a new thought.
	if (m_newThought == nullptr || !m_newThought->isVisible()) {
		clearAnchor();
	}
}

void CanvasWidget::onAnchorMoved(QPoint point) {
	if (m_anchorSource == nullptr)
		return;

	const QSize highlightSize(20, 20);
	// Position of the source anchor.
	AnchorPoint sourcePos = m_anchorSource->widget->getAnchorFrom(
		m_anchorSource->type
	);

	// Move higlight.
	m_anchorHighlight.setGeometry(
		point.x() - highlightSize.width() / 2,
		point.y() - highlightSize.height() / 2,
		highlightSize.width(),
		highlightSize.height()
	);

	// Move "new thought" widget to the center of anchor.
	const QSize defaultSize = m_layout->defaultWidgetSize();
	if (m_newThought != nullptr) {
		m_newThought->setGeometry(
			point.x() - defaultSize.width() / 2,
			point.y() - defaultSize.height() / 2,
			defaultSize.width(),
			defaultSize.height()
		);
	}

	// If distance is enough, show new thought widget, otherwise show anchor.
	if (
		std::abs(point.x() - sourcePos.x) < minAnchorDistance &&
		std::abs(point.y() - sourcePos.y) < minAnchorDistance
	) {
		m_newThought->hide();
		m_anchorHighlight.show();
	} else if (
		auto *under = widgetUnder(point);
		under != nullptr && under != m_anchorSource->widget
	) {
		if (m_overThought == nullptr) {
			m_overThought = under;
			m_newThought->hide();
			m_anchorHighlight.hide();
			m_overThought->setHighlight(true);
		}
	} else {
		if (m_overThought != nullptr) {
			m_overThought->setHighlight(false);
			m_overThought = nullptr;
		}
		m_newThought->show();
		m_anchorHighlight.hide();
	}

	// Redraw.
	update();
}

void CanvasWidget::onAnchorCanceled() {
	clearAnchor();
}

void CanvasWidget::onAnchorReleased(QPoint centerPos) {
	if (m_anchorSource == nullptr)
		return;

	// Position of the dragged anchor.
	QRect highlightRect = m_anchorHighlight.geometry();
	// Position of the source anchor.
	AnchorPoint sourcePos = m_anchorSource->widget->getAnchorFrom(
		m_anchorSource->type
	);

	// Don't create new thought if the diff between current point and source is
	// too small.
	if (
		std::abs(centerPos.x() - sourcePos.x) < minAnchorDistance &&
		std::abs(centerPos.y() - sourcePos.y) < minAnchorDistance
	) {
		return clearAnchor();
	}

	if (m_overThought != nullptr) {
		updateConnection();
	} else if (m_newThought != nullptr) {
		setupNewThought();
	}
}

void CanvasWidget::onTextConfirmed(
	ThoughtWidget *source,
	QString text,
	std::function<void(bool)> callback
) {
	ThoughtId id = source->id();
	emit textChanged(id, text, callback);
}

// Creation slots.

void CanvasWidget::onSuggestionSelected(ThoughtId to, QString name) {
	ThoughtId from = m_anchorSource->widget->id();
	ConnectionType type;

	switch (m_anchorSource->type) {
		case AnchorType::AnchorLink:
			type = ConnectionType::link;
			break;
		case AnchorType::AnchorParent:
			{
				ThoughtId tmp = to;
				to = from;
				from = tmp;
				type = ConnectionType::child;
				break;
			}
		case AnchorType::AnchorChild:
			type = ConnectionType::child;
			break;
	}

	emit thoughtConnected(from, to, type, [this](bool success){
		if (success) {
			this->clearAnchor();
		}
	});
}

void CanvasWidget::onNewThoughtTextChanged(ThoughtWidget *widget) {
	// Relayout.
	onWidgetActivated(widget);
	// Emit text update.
	emit newThoughtTextChanged(QString::fromStdString(widget->text()));
}

void CanvasWidget::onCreateCanceled(ThoughtWidget *widget) {
	clearAnchor();
	hideSuggestions();
}

void CanvasWidget::onCreateConfirmed(
	ThoughtWidget *widget,
	QString text,
	std::function<void(bool)> checkCallback
) {
	if (m_anchorSource == nullptr)
		return;

	if (text.isEmpty()) {
		onCreateCanceled(widget);
		return;
	}

	if (
		m_suggestions != nullptr &&
		m_suggestions->items().size() > 0 &&
		m_suggestions->selectedIndex() != -1 &&
		m_suggestions->selectedIndex() < m_suggestions->items().size()
	) {
		ConnectionItem item = m_suggestions->items()[m_suggestions->selectedIndex()];
		onSuggestionSelected(item.id, item.name);
		return;
	}

	auto onCreateCallback = [this, widget, checkCallback](bool result, ThoughtId id){
		checkCallback(result);
		if (result) {
			// Connect signals.
			disconnect(widget, nullptr, this, nullptr);
			this->connectWidget(widget);
			// Set widget's ID and add it to the list of widgets.
			widget->setId(id);
			widget->setAnchorsActive(true);
			this->m_widgets.insert_or_assign(id, widget);
			this->m_newThought = nullptr;
			// Clear editing widgets and connections.
			this->clearAnchor();
			this->hideSuggestions();
		}
	};

	emit thoughtCreated(
		m_anchorSource->widget->id(),
		m_anchorSource->type == AnchorType::AnchorLink
			? ConnectionType::link
			: ConnectionType::child,
		m_anchorSource->type == AnchorType::AnchorParent
			? true
			: false,
		text,
		onCreateCallback
	);
}

void CanvasWidget::onNextSuggestion() {
	if (m_suggestions != nullptr && m_suggestions->isVisible())
		m_suggestions->onNextItem();
}

void CanvasWidget::onPrevSuggestion() {
	if (m_suggestions != nullptr && m_suggestions->isVisible())
		m_suggestions->onPrevItem();
}

void CanvasWidget::onMenuRequested(
	ThoughtWidget *widget,
	const QPoint& point
) {
	if (m_layout == nullptr)
		return;
	if (!widget->canDelete())
		return;

	m_menuThought = widget;

	QMenu contextMenu(this);

	if (m_style != nullptr) {
		contextMenu.setStyleSheet(m_style->menuStyle());
	}

	QString thoughtName = QString::fromStdString(m_menuThought->text());
	if (thoughtName.size() > 15)
		thoughtName = thoughtName.left(12) + "...";

	QString forgetMenu = tr("Forget \"%1\"").arg(thoughtName);
	QAction action(forgetMenu, this);
	connect(&action, SIGNAL(triggered()), this, SLOT(onDeleteThought()));
	connect(&contextMenu, SIGNAL(destroyed()), SLOT(onMenuHide()));

	contextMenu.addAction(&action);
	contextMenu.exec(mapToGlobal(point));
}

void CanvasWidget::onMenuHide() {
	// Do nothing for now.
	m_menuThought = nullptr;
}

void CanvasWidget::onDeleteThought() {
	if (m_menuThought == nullptr) {
		return;
	}

	emit thoughtDeleted(m_menuThought->id());
	m_menuThought = nullptr;
}

void CanvasWidget::onDisconnect() {
	if (!m_pathHighlight.has_value()) {
		return;
	}

	Path path = m_pathHighlight.value();
	if (path.from == nullptr || path.to == nullptr) {
		return;
	}

	emit thoughtsDisconnected(path.from->id(), path.to->id());

	// Cleanup.
	m_pathHighlight = std::nullopt;
	update();
}

// Helpers.

void CanvasWidget::clearAnchor() {
	m_anchorHighlight.hide();
	m_overlay.setParent(nullptr);

	if (m_newThought != nullptr) {
		delete m_newThought;
		m_newThought = nullptr;
	}

	if (m_anchorSource != nullptr) {
		delete m_anchorSource;
		m_anchorSource = nullptr;
	}

	if (m_overThought != nullptr) {
		m_overThought->setHighlight(false);
		m_overThought = nullptr;
	}

	update();
}

inline AnchorType CanvasWidget::reverseAnchorType(AnchorType type) {
	switch (type) {
		case AnchorType::AnchorParent:
			return AnchorType::AnchorChild;
		case AnchorType::AnchorChild:
			return AnchorType::AnchorParent;
		case AnchorType::AnchorLink:
			return AnchorType::AnchorLink;
	}

	// Should not reach here.
	assert(false);
	return AnchorType::AnchorLink;
}

ThoughtWidget *CanvasWidget::widgetUnder(QPoint point) {
	QRect geometry;
	std::unordered_map<ThoughtId, ThoughtWidget*>::iterator it;

	for (it = m_widgets.begin(); it != m_widgets.end(); it++) {
		if (it->second->parent() == nullptr)
			continue;

		geometry = it->second->geometry();
		if (
			point.x() >= geometry.x() &&
			point.x() <= geometry.x() + geometry.width() &&
			point.y() >= geometry.y() &&
			point.y() <= geometry.y() + geometry.height()
		) {
			return it->second;
		}
	}

	return nullptr;
}

void CanvasWidget::setupNewThought() {
	assert(m_newThought != nullptr);
	// Activate overlay.
	m_overlay.setParent(this);
	m_overlay.setGeometry(rect());
	m_overlay.show();
	m_overlay.raise();
	// Activate new thought widget.
	m_newThought->setAttribute(Qt::WA_TransparentForMouseEvents, false);
	m_newThought->raise();
	m_newThought->activate();
}

void CanvasWidget::updateConnection() {
	assert(m_overThought != nullptr);
	assert(m_anchorSource != nullptr);

	ThoughtId from = m_anchorSource->widget->id();
	ThoughtId to = m_overThought->id();
	ConnectionType type;

	switch (m_anchorSource->type) {
		case AnchorType::AnchorLink:
			type = ConnectionType::link;
			break;
		case AnchorType::AnchorParent:
			type = ConnectionType::child;
			to = from;
			from = m_overThought->id();
			break;
		case AnchorType::AnchorChild:
			type = ConnectionType::child;
			break;
	}

	emit thoughtConnected(from, to, type, [](bool){});
}

void CanvasWidget::layoutSuggestions() {
	const int padding = 5;
	const int width = 200;

	if (m_newThought == nullptr || m_suggestions == nullptr)
		return;

	QSize hint = m_suggestions->sizeHint();
	QRect newRect = m_newThought->geometry();
	int height = hint.height() + padding * 2;

	int x = std::min(
		newRect.x() + (newRect.width() - width) / 2,
		rect().width() - width
	);
	x = std::max(x, 0);

	int y = newRect.y() + newRect.height();
	if ((y + height) > rect().height()) {
		y = newRect.y() - height;
	}

	m_suggestions->setGeometry(
		padding, padding,
		width - padding * 2, hint.height()
	);

	m_suggestionsContainer->setGeometry(
		x, y,
		width, hint.height() + padding * 2
	);
	m_suggestionsContainer->show();

	update();
}

