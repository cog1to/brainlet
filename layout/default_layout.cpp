#include "layout/default_layout.h"
#include "layout/item_layout.h"

DefaultLayout::DefaultLayout(Style* style)
	: BaseLayout(style) {}

void DefaultLayout::reload() {
	if (m_state == nullptr) {
		return;
	}

	const Thought *thought = m_state->centralThought();
	if (thought == nullptr) {
		return;
	}

	// Clear the previous layout.
	m_layout.clear();

	// Recalculate widget positions.
	updateWidgets();
}

void DefaultLayout::setSize(QSize size) {
	m_size = size;
	updateWidgets();
}

void DefaultLayout::updateWidgets() {
	if (m_state == nullptr) {
		return;
	}

	const Thought *thought = m_state->centralThought();
	if (thought == nullptr) {
		return;
	}

	// Position the central element.
	QSize centralSize = widgetSize(thought->name(), s_centerWidgetSize);
	ItemLayout centralLayout = ItemLayout(
		(m_size.width() - centralSize.width()) / 2.0,
		(m_size.height() - centralSize.height()) / 2.0,
		centralSize.width(),
		centralSize.height(),
		true
	);

	// Save the central element.
	m_layout.insert_or_assign(thought->id(), centralLayout);

	if (thought->links().size() > 0) {
		layoutVerticalSide(
			thought->links(),
			QRect(
				(int)((float)m_size.width() * (s_sidePanelWidth - s_widgetSize)),
				(int)((float)m_size.height() * s_sidePanelWidth),
				(int)((float)m_size.width() * s_widgetSize),
				(int)((float)m_size.height() * (1.0 - s_sidePanelWidth * 2.0))
			)
		);
	}
}

bool compare_thoughts(Thought *a, Thought *b) {
	return (a->name().compare(b->name()) < 0);
}

void DefaultLayout::layoutVerticalSide(
	const std::vector<ThoughtId>& thoughtIds,
	QRect rect
) {
	// Ordered array is required to display widgets alphabetically.
	std::vector<Thought*> sorted;
	for (const auto& id: thoughtIds) {
		if (auto found = m_state->thoughts()->find(id); found != m_state->thoughts()->end()) {
			sorted.push_back(found->second);
		}
	}
	std::sort(sorted.begin(), sorted.end(), compare_thoughts);

	// Calculate all sized in order.
	std::vector<QSize> sizes;
	for (const auto thought: sorted) {
		QSize size = widgetSize(thought->name(), s_widgetSize);
		sizes.push_back(size);
	}

	// Generate layouts. We have 3 options:
	// 1 widget - center vertically
	// 2 widgets - space evenly from top and bottom
	// 3+ widgets - space evenly inbetween
	if (sizes.size() == 1) {
		QSize size = sizes[0];
		ItemLayout layout(
			rect.x() + (rect.width() - size.width()) / 2.0,
			rect.y() + (rect.height() - size.height()) / 2.0,
			size.width(),
			size.height(),
			true
		);
		m_layout.insert_or_assign(sorted[0]->id(), layout);
	} else if (sizes.size() == 2) {
		QSize first = sizes[0];
		QSize second = sizes[1];
		int spacer = (rect.height() - first.height() - second.height()) / 3;

		ItemLayout firstLayout(
			rect.x() + (rect.width() - first.width()) / 2.0,
			rect.y() + spacer,
			first.width(),
			first.height(),
			true
		);
		m_layout.insert_or_assign(sorted[0]->id(), firstLayout);

		ItemLayout secondLayout(
			rect.x() + (rect.width() - second.width()) / 2.0,
			rect.y() + rect.height() - spacer - second.height(),
			second.width(),
			second.height(),
			true
		);
		m_layout.insert_or_assign(sorted[1]->id(), secondLayout);
	} else {
		// Calculate spacer height by subtracting total height of elements from
		// available height.
		auto reducer = [](int acc, const QSize& size) { return acc + size.height(); };
		int totalHeight = std::accumulate(sizes.begin(), sizes.end(), 0, reducer);
		int spacer = (rect.height() - totalHeight) / (sizes.size() - 1);

		int y = rect.y(), idx;
		for (idx = 0; idx < sizes.size(); idx++) {
			QSize size = sizes[idx];
			Thought *thought = sorted[idx];

			ItemLayout layout(
				rect.x() + (rect.width() - size.width()) / 2.0, y,
				size.width(),
				size.height(),
				true
			);

			m_layout.insert_or_assign(thought->id(), layout);
			y += size.height() + spacer;
		}
	}
}

QSize DefaultLayout::widgetSize(std::string text, float maxWidthRatio) {
	m_template.setText(text);

	// Get size hint to estimate the full text length.
	QSize sizeHint = m_template.sizeHint();
	// Actual width is the width of full text up to max allowed width.
	int actualWidth = std::min(
		(int)((float)(m_size.width()) * maxWidthRatio),
		sizeHint.width()
	);

	return QSize(
		actualWidth,
		sizeHint.height()
	);
}

const std::unordered_map<ThoughtId, ItemLayout>* DefaultLayout::items() const {
	return &m_layout;
}

