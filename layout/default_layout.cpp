#include "layout/default_layout.h"
#include "layout/item_layout.h"
#include "layout/scroll_area_layout.h"

DefaultLayout::DefaultLayout(Style* style)
	: BaseLayout(style)
{
	// Sample text to estimate "average" widget width.
	m_template.setText("xxxxxxxxxx");

	QSize size = m_template.sizeHint();
	m_widgetHeight = size.height();
	m_verticalWidgetWidth = size.width();
	m_widgetSpacing = size.height() / 2;
	m_sidePadding = style->scrollWidth();
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
	m_scrollAreas.clear();

	// Recalculate widget positions.
	updateWidgets();
}

void DefaultLayout::setSize(QSize size) {
	m_size = size;
	m_topSideHeight = (int)((float)size.height() * s_sideRatio);
	m_leftSideWidth = (int)((float)size.width() * s_sideRatio);
	reload();
}

void DefaultLayout::setState(State *state) {
	m_state = state;
	loadSiblings();
	reload();
}

void DefaultLayout::loadSiblings() {
	if (m_state == nullptr) {
		return;
	}

	const Thought *thought = m_state->centralThought();
	if (thought == nullptr) {
		return;
	}

	const std::unordered_map<ThoughtId, Thought*> *thoughts = m_state->thoughts();

	// Order direct connected nodes.
	sortNodes(m_parents, thought->parents(), thoughts);
	sortNodes(m_links, thought->links(), thoughts);
	sortNodes(m_children, thought->children(), thoughts);

	// Gather siblings.
	m_siblings.clear();
	for (const auto *parent: m_parents) {
		for (const auto& id: parent->children()) {
			if (id == thought->id())
				continue;
			if (auto found = thoughts->find(id); found != thoughts->end())
				m_siblings.push_back(found->second);
		}
	}
	std::sort(m_siblings.begin(), m_siblings.end(), compareThoughts);
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
		(m_size.width() - centralSize.width()) / 2,
		(m_size.height() - centralSize.height()) / 2,
		centralSize.width(),
		centralSize.height(),
		true
	);

	// Save the central element.
	m_layout.insert_or_assign(thought->id(), centralLayout);

	//  Left side.
	if (m_links.size() > 0) {
		layoutVerticalSide(
			m_links,
			QRect(
				m_sidePadding,
				m_sidePadding + m_topSideHeight,
				m_leftSideWidth,
				m_size.height() - (m_topSideHeight + m_sidePadding) * 2
			),
			ScrollBarPos::Left
		);
	}

	// Top side.
	if (m_parents.size() > 0) {
		layoutHorizontalSide(
			m_parents,
			QRect(
				m_sidePadding + m_leftSideWidth,
				m_sidePadding,
				m_size.width() - (m_leftSideWidth + m_sidePadding) * 2,
				m_topSideHeight
			),
			ScrollBarPos::Top
		);
	}

	// Bottom side.
	if (m_children.size() > 0) {
		layoutHorizontalSide(
			m_children,
			QRect(
				m_sidePadding + m_leftSideWidth,
				m_size.height() - m_topSideHeight - m_sidePadding,
				m_size.width() - (m_leftSideWidth + m_sidePadding) * 2,
				m_topSideHeight
			),
			ScrollBarPos::Bottom
		);
	}

	// Right side.
	if (m_siblings.size() > 0) {
		layoutVerticalSide(
			m_siblings,
			QRect(
				m_size.width() - m_leftSideWidth - m_sidePadding,
				m_sidePadding + m_topSideHeight,
				m_leftSideWidth,
				m_size.height() - (m_topSideHeight + m_sidePadding) * 2
			),
			ScrollBarPos::Right
		);
	}
}

void DefaultLayout::layoutVerticalSide(
	const std::vector<Thought*>& sorted,
	QRect rect,
	ScrollBarPos scrollPos
) {
	// Calculate all sized in order.
	std::vector<QSize> sizes;
	for (const auto thought: sorted) {
		QSize size = widgetSize(thought->name(), m_leftSideWidth);
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
			spacer = (rect.height() - totalHeight) / (maxCount - 1);
			totalSpaces = spacer * (maxCount - 1);
		}
	}

	// Apply offset. If the number of visible rows has changed, the cached
	// offset value might invalid, i.e. trying to layout non-existent rows.
	// So if number of visible rows after offset goes beyond total row count,
	// we reduce the offset to fit into the layout.
	auto cachedOffset = m_offsets.find(scrollPos);
	int offset = 0;
	if (cachedOffset != m_offsets.end()) {
		if (cachedOffset->second + maxCount > sorted.size()) {
			offset = sorted.size() - maxCount;
		} else {
			offset = cachedOffset->second;
		}
		m_offsets.insert_or_assign(scrollPos, offset);
	}

	// Layout.
	int y = rect.y() + (rect.height() - totalHeight - totalSpaces) / 2, idx;
	for (idx = offset; idx < maxCount + offset; idx++) {
		QSize size = sizes[idx];
		Thought *thought = sorted[idx];

		ItemLayout layout(
			rect.x() + (rect.width() - size.width()) / 2, y,
			size.width(),
			size.height(),
			true
		);

		m_layout.insert_or_assign(thought->id(), layout);
		y += size.height() + spacer;
	}

	// We have items left outside, add scroll area.
	if (maxCount < sorted.size()) {
		int scrollWidth = m_style->scrollWidth();
		float barWidth = (float)maxCount / (float)sorted.size();
		float relativeOffset = maxCount != 0 ? ((float)offset / (float)sorted.size()) : 0;

		ScrollAreaLayout layout(
			rect.x() - scrollWidth,
			rect.y(),
			rect.width() + scrollWidth,
			rect.height(),
			scrollPos,
			barWidth,
			relativeOffset,
			sorted.size() - maxCount
		);
		m_scrollAreas.insert_or_assign(scrollPos, layout);
	}
}

void DefaultLayout::layoutHorizontalSide(
	const std::vector<Thought*>& sorted,
	QRect rect,
	ScrollBarPos scrollPos
) {
	// Total column count from available space.
	int visibleColumnCount, w = m_verticalWidgetWidth;
	for (visibleColumnCount = 0; w < rect.width(); visibleColumnCount++) {
		w += m_verticalWidgetWidth;
	}

	// Can't fit a single column.
	if (visibleColumnCount == 0)
		return;

	// Number of items per column.
	int columnCapacity = rect.height() / m_widgetHeight;

	// Can't fit a single row.
	if (columnCapacity == 0)
		return;

	// Number of columns required to fit all nodes.
	int requiredColumnCount = (sorted.size() + columnCapacity - 1) / columnCapacity;

	// We have two cases:
	// 1. All items fit into the visible area, in which case we try layout items
	//    to have equal number of items in each column.
	// 2. There's more items than slots available in the visible area, in which
	//    case we try to display as much items as possible in the visible area.
	int itemsPerColumn = 0, columnCount = 0;
	if (requiredColumnCount <= visibleColumnCount) {
		columnCount = (sorted.size() + columnCapacity - 1) / columnCapacity;
		itemsPerColumn = (sorted.size() + columnCount - 1) / columnCount;
	} else {
		itemsPerColumn = columnCapacity;
		columnCount = visibleColumnCount;
	}

	int columnWidth = rect.width() / columnCount;

	// Apply offset. If the number of visible columns has changed, the cached
	// offset value might invalid, i.e. trying to layout non-existent columns.
	// So if number of visible columns after offset goes beyond total column
	// count, we reduce the offset to fit into the layout.
	auto cachedOffset = m_offsets.find(scrollPos);
	int offset = 0;
	if (cachedOffset != m_offsets.end()) {
		if (cachedOffset->second + columnCount > requiredColumnCount) {
			offset = requiredColumnCount - columnCount;
		} else {
			offset = cachedOffset->second;
		}
		m_offsets.insert_or_assign(scrollPos, offset);
	}

	// Layout columns.
	int idx = offset * itemsPerColumn, col, row, x, y, height, rowCount;
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
			QSize size = widgetSize(
				thought->name(),
				std::max(columnWidth - 2 * m_widgetSpacing, m_verticalWidgetWidth)
			);

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

	// We have items left outside, add scroll area.
	if (columnCount < requiredColumnCount) {
		int scrollWidth = m_style->scrollWidth();
		float barWidth = (float)columnCount / (float)requiredColumnCount;
		float relativeOffset = requiredColumnCount != 0
			? ((float)offset / (float)requiredColumnCount)
			: 0;

		ScrollAreaLayout layout(
			rect.x(),
			rect.y() - scrollWidth,
			rect.width(),
			rect.height() + scrollWidth,
			scrollPos,
			barWidth,
			relativeOffset,
			requiredColumnCount - columnCount
		);
		m_scrollAreas.insert_or_assign(scrollPos, layout);
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

const std::unordered_map<unsigned int, ScrollAreaLayout>* DefaultLayout::scrollAreas() const {
	return &m_scrollAreas;
}

// Utility functions.

inline bool DefaultLayout::compareThoughts(Thought *a, Thought *b) {
	return (a->name().compare(b->name()) < 0);
}

inline void DefaultLayout::sortNodes(
	std::vector<Thought*>& list,
	const std::vector<ThoughtId>& ids,
	const std::unordered_map<ThoughtId, Thought*>* map
) {
	list.clear();
	for (const auto& id: ids) {
		if (auto found = map->find(id); found != map->end()) {
			list.push_back(found->second);
		}
	}
	std::sort(list.begin(), list.end(), compareThoughts);
}

// Slots.

void DefaultLayout::onScroll(unsigned int scrollId, int change) {
	int offset = 0;

	auto layout = m_scrollAreas.find(scrollId);
	if (layout == m_scrollAreas.end())
		return;

	auto cached = m_offsets.find(scrollId);
	if (cached != m_offsets.end()) {
		offset = cached->second;
	}

	offset = std::max(0, std::min(offset + change, layout->second.maxNodeOffset));
	m_offsets.insert_or_assign(scrollId, offset);

	// TODO: Optimize. Doing full reload on changing one scroll area is wasting
	// time.
	reload();
}
