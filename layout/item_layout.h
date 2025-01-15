#ifndef H_ITEM_LAYOUT
#define H_ITEM_LAYOUT

class ItemLayout {
public:
	ItemLayout(int x, int y, int w, int h, bool visible);
	int x, y, w, h;
	bool visible;
};

#endif
