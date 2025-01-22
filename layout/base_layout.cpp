#include <unordered_map>

#include "model/thought.h"
#include "layout/base_layout.h"
#include "widgets/style.h"

BaseLayout::BaseLayout(Style *style)
 : m_template(nullptr, style, 0, true, "", false, false, false)
{
	m_style = style;
}

void BaseLayout::setState(State* state) {
	m_state = state;
	reload();
}

void BaseLayout::setSize(QSize size) {
	m_size = size;
	reload();
}

void BaseLayout::setStyle(Style* style) {
	m_style = style;
	reload();
}

void BaseLayout::onScroll(unsigned int, int) {}
