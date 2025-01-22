#include "layout/scroll_area_layout.h"

ScrollAreaLayout::ScrollAreaLayout(
	int _x, int _y,
	int _w, int _h,
	ScrollBarPos _pos,
	float _barWidth,
	float _offset,
	int _max
) {
	x = _x;
	y = _y;
	w = _w;
	h = _h;
	pos = _pos;
	barWidth = _barWidth;
	offset = _offset;
	maxNodeOffset = _max;
}

