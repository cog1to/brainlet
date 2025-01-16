#include <assert.h>

#include <QColor>
#include <QWidget>

#include "layout/base_layout.h"
#include "widgets/base_canvas_widget.h"

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
	// TODO
}

void BaseCanvasWidget::updateLayout() {
	if (m_layout == nullptr || m_state == nullptr) {
		return;
	}

	// Central thought. TODO: remove this, we should only use main map ideally.
	const Thought *main = m_state->centralThought();
	const std::unordered_map<ThoughtId, Thought*>* thoughts = m_state->thoughts();

	// Debug implementation. Will need some rethinking and reworking later.
	const std::unordered_map<ThoughtId, ItemLayout> *items = m_layout->items();

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

		m_widgets.insert_or_assign(it->first, widget);

		const ItemLayout& layout = it->second;
		widget->setGeometry(
			layout.x, layout.y,
			layout.w, layout.h
		);

		widget->setParent(this);
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
