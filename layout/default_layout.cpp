#include <iostream>

#include "layout/default_layout.h"
#include "layout/item_layout.h"

DefaultLayout::DefaultLayout(Style* style)
	: BaseLayout(style)
{
	m_template.setText("xxxxxxxxxx");

	QSize size = m_template.sizeHint();
	m_widgetWidth = size.width();
	m_widgetHeight = size.height();
	m_widgetSpacing = size.height() / 2;
}

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
	reload();
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
	QSize centralSize = widgetSize(thought->name(), m_size.width() * 0.4);
	ItemLayout centralLayout = ItemLayout(
		(m_size.width() - centralSize.width()) / 2.0,
		(m_size.height() - centralSize.height()) / 2.0,
		centralSize.width(),
		centralSize.height(),
		true
	);

	// Save the central element.
	m_layout.insert_or_assign(thought->id(), centralLayout);

	//  Left side.
	if (thought->links().size() > 0) {
		layoutVerticalSide(
			thought->links(),
			QRect(
				m_sidePadding,
				m_widgetWidth + m_sidePadding,
				m_widgetWidth,
				m_size.height() - (m_widgetWidth + m_sidePadding) * 2
			)
		);
	}

	// Top side.
	if (thought->parents().size() > 0) {
		layoutHorizontalSide(
			thought->parents(),
			QRect(
				m_widgetWidth + m_sidePadding,
				m_sidePadding,
				m_size.width() - (m_widgetWidth + m_sidePadding) * 2,
				m_widgetWidth
			)
		);
	}

	// Bottom side.
	if (thought->children().size() > 0) {
		layoutHorizontalSide(
			thought->children(),
			QRect(
				m_widgetWidth + m_sidePadding,
				m_size.height() - m_widgetWidth - m_sidePadding,
				m_size.width() - (m_widgetWidth + m_sidePadding) * 2,
				m_widgetWidth
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
		QSize size = widgetSize(thought->name(), m_widgetWidth);
		sizes.push_back(size);
	}

	// Total height = height of all widget + (count of widgets - 1) * spacer.
	auto reducer = [](int acc, const QSize& size) { return acc + size.height(); };
	int totalSpaces = m_widgetSpacing * (sizes.size() - 1);
	int totalHeight = std::accumulate(sizes.begin(), sizes.end(), 0, reducer);
	int spacer = m_widgetSpacing;
	int maxCount = sizes.size();

	// If we can't fit all widgets, we fit max number of them.
	if ((totalHeight + totalSpaces) > rect.height()) {
		if (totalHeight <= rect.height()) {
			spacer = (rect.height() - totalHeight) / (sizes.size() - 1);
			totalSpaces = spacer * (sizes.size() - 1);
		} else {
			while (totalHeight > rect.height()) {
				maxCount -= 1;
				totalHeight -= sizes[maxCount].height();
			}
			spacer = (rect.height() - totalHeight) / (sizes.size() - 1);
			totalSpaces = spacer * (sizes.size() - 1);
		}
	}

	// Layout.
	int y = rect.y() + (rect.height() - totalHeight - totalSpaces) / 2, idx;
	for (idx = 0; idx < maxCount; idx++) {
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

void DefaultLayout::layoutHorizontalSide(
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

	// Total column count from available space.
	int visibleColumnCount, w = m_widgetWidth;
	for (visibleColumnCount = 0; w < rect.width(); visibleColumnCount++) {
		w += m_widgetWidth;
	}

	// Can't fit a single column.
	if (visibleColumnCount == 0)
		return;

	// Number of items per column.
	int columnCapacity = rect.height() / m_widgetHeight;

	// Can't fit a single row.
	if (columnCapacity == 0)
		return;

	int requiredColumnCount = (sorted.size() + visibleColumnCount - 1) / visibleColumnCount;

	// We have two cases:
	// 1. All items fit into the visible area, in which case we try layout items
	//    to have equal number of items in each column.
	// 2. There's more items than slots available in the visible area, in which
	//    case we try to display as much items as possible in the visible area.
	int itemsPerColumn = 0, columnCount = 0;
	if (requiredColumnCount <= columnCapacity) {
		itemsPerColumn = requiredColumnCount;
		columnCount = (sorted.size() + itemsPerColumn - 1) / itemsPerColumn;
	} else {
		itemsPerColumn = columnCapacity;
		columnCount = visibleColumnCount;
	}

	int columnWidth = rect.width() / columnCount;

	// Layout columns.
	int idx = 0, col, row, x, y, height, rowCount;
	for (col = 0; col < columnCount; col++) {

		// Horizontal and vertical position of the column.
		x = rect.x() + (columnWidth * col);
		y = rect.y();

		// Height of the column content: items in column * widget height.
		rowCount = std::min(itemsPerColumn, (int)(sorted.size()) - idx);
		height = rowCount * m_widgetHeight;

		// Layout each item in the column.
		for (row = 0; row < rowCount; row++) {
			Thought *thought = sorted[idx];
			QSize size = widgetSize(thought->name(), std::max(columnWidth - 2 * m_widgetSpacing, m_widgetWidth));

			ItemLayout layout(
				x + (columnWidth - size.width()) / 2,
				y + ((rect.height() - height) / 2) + (row * m_widgetHeight),
				size.width(),
				size.height(),
				true
			);

			m_layout.insert_or_assign(thought->id(), layout);

			idx += 1;
		}
	}
}

QSize DefaultLayout::widgetSize(std::string text, int maxWidth) {
	m_template.setText(text);

	// Get size hint to estimate the full text length.
	QSize sizeHint = m_template.sizeHint();
	// Actual width is the width of full text up to max allowed width.
	int actualWidth = std::min(
		maxWidth,
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

