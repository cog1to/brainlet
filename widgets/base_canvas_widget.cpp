#include <assert.h>

#include <QColor>
#include <QWidget>

#include "layout/base_layout.h"
#include "layout/scroll_area_layout.h"
#include "widgets/base_canvas_widget.h"
#include "widgets/scroll_area_widget.h"
#include "widgets/thought_widget.h"

BaseCanvasWidget::BaseCanvasWidget(
	QWidget *parent,
	Style *style,
	BaseLayout *layout
) : BaseWidget(parent, style) {
	m_layout = layout;

	setStyleSheet(
		QString("background-color: %1").arg(
			style->background().name(QColor::HexRgb)
		)
	);
}

void BaseCanvasWidget::setState(State *state) {
	m_state = state;
	if (m_layout != nullptr) {
		m_layout->setState(state);
	}

	updateLayout();
}

void BaseCanvasWidget::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event);

	if (m_layout != nullptr) {
		m_layout->setSize(event->size());
	}

	updateLayout();
}

void BaseCanvasWidget::paintEvent(QPaintEvent *) {
	// TODO: connection lines.
}

void BaseCanvasWidget::updateLayout() {
	if (m_layout == nullptr || m_state == nullptr) {
		return;
	}

	// Central thought. TODO: remove this, we should only use main map ideally.
	const Thought *main = m_state->centralThought();
	const std::unordered_map<ThoughtId, Thought*>* thoughts = m_state->thoughts();
	const std::unordered_map<ThoughtId, ItemLayout> *items = m_layout->items();

	// Layout scroll areas first.
	layoutScrollAreas();

	// Layout all widgets.
	std::unordered_map<ThoughtId, ItemLayout>::const_iterator it;
	for (it = items->begin(); it != items->end(); it++) {
		ThoughtWidget *widget = cachedWidget(it->first);

		// TODO: remove this. relying on central/non-central is not good.
		if (main->id() == it->first) {
			if (widget == nullptr) {
				widget = createWidget(main, false);
			}
		} else if (auto found = thoughts->find(it->first); found != thoughts->end()) {
			if (widget == nullptr) {
				widget = createWidget(found->second, true);
			}
		}

		// Save the widget to the cache.
		m_widgets.insert_or_assign(it->first, widget);

		// Place the widget in the canvas.
		const ItemLayout& layout = it->second;
		widget->setGeometry(
			layout.x, layout.y,
			layout.w, layout.h
		);

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
		if (wit->first == main->id()) {
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
			m_scrollAreas.insert_or_assign(it->first, widget);
		}

		widget->setScrollBarPos(layout.pos);

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
	ScrollAreaWidget *widget = new ScrollAreaWidget(this,	m_style, pos);

	// TODO: connect signals.

	return widget;
}

ThoughtWidget *BaseCanvasWidget::cachedWidget(ThoughtId id) {
	if (auto search = m_widgets.find(id); search != m_widgets.end()) {
		return search->second;
	}
	return nullptr;
}

ThoughtWidget *BaseCanvasWidget::createWidget(
	const Thought *thought,
	bool readonly
) {
	ThoughtWidget *widget = new ThoughtWidget(
		this,
		m_style,
		thought->id(),
		readonly,
		thought->name(),
		thought->hasParents(), thought->hasChildren(), thought->hasLinks()
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
