#ifndef H_ITEM_LAYOUT
#define H_ITEM_LAYOUT

#include <string>

#include "model/thought.h"

class ItemLayout {
public:
	ItemLayout(
		ThoughtId id,
		const std::string *name,
		int x, int y,
		int w, int h,
		bool visible,
		bool hasParents, bool hasChildren, bool hasLinks,
		bool rightSideLink
	);
	ThoughtId id;
	const std::string *name;
	int x, y, w, h;
	bool visible;
	bool hasParents;
	bool hasChildren;
	bool hasLinks;
	bool rightSideLink;
};

#endif

