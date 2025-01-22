#ifndef H_SCROLL_AREA_LAYOUT
#define H_SCROLL_AREA_LAYOUT

enum ScrollBarPos {
	Top,
	Bottom,
	Left,
	Right
};

class ScrollAreaLayout {
public:
	ScrollAreaLayout(
		int x, int y,
		int w, int h,
		ScrollBarPos pos,
		float barWidth,
		float offset,
		int maxNodeOffset
	);
	int x, y, w, h;
	ScrollBarPos pos;
	float barWidth, offset;
	int maxNodeOffset;
};

#endif

