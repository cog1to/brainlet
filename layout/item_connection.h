#ifndef H_ITEM_CONNECTION_LAYOUT
#define H_ITEM_CONNECTION_LAYOUT

#include "model/thought.h"

enum ConnectionType { link, child };

typedef struct {
	ThoughtId from;
	ThoughtId to;
	ConnectionType type;
} ItemConnection;

#endif
