#include "layout/item_layout.h"

ItemLayout::ItemLayout(
	ThoughtId _id,
	const std::string *_name,
	int _x, int _y,
	int _w, int _h,
	bool _visible,
	bool _hasParents, bool _hasChildren, bool _hasLinks,
	bool _rightSideLink,
	bool _canDelete
) {
	id = _id;
	name = _name;
	x = _x;
	y = _y;
	w = _w;
	h = _h;
	visible = _visible;
	hasParents = _hasParents;
	hasChildren = _hasChildren;
	hasLinks = _hasLinks;
	rightSideLink = _rightSideLink;
	canDelete = _canDelete;
}
