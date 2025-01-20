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
		ScrollBarPos pos
	);
	int x, y, w, h;
	ScrollBarPos pos;
};

#endif

