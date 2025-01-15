#ifndef H_ITEM_LAYOUT
#define H_ITEM_LAYOUT

#include "model/thought.h"

enum ConnectionType { parent, child, link };

typedef struct {
	ThoughtId from;
	ThoughtId to;
	ConnectionType type;
}

#endif
