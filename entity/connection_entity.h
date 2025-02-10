#ifndef H_CONNECTION_ENTITY
#define H_CONNECTION_ENTITY

#include "model/thought.h"

class ConnectionEntity {
public:
	ConnectionEntity(ThoughtId _from, ThoughtId _to, ConnectionType _t)
		: from(_from), to(_to), type(_t) {};
	ThoughtId from;
	ThoughtId to;
	ConnectionType type;
};

#endif

