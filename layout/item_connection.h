#ifndef H_ITEM_CONNECTION_LAYOUT
#define H_ITEM_CONNECTION_LAYOUT

#include "model/thought.h"

typedef struct {
	ThoughtId from;
	ThoughtId to;
	ConnectionType type;
} ItemConnection;

#endif
