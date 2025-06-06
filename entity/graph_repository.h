#ifndef H_GRAPH_REPOSITORY
#define H_GRAPH_REPOSITORY

#include "model/model.h"

struct CreateResult {
	bool success;
	ThoughtId id;
	CreateResult(bool _success, ThoughtId _id): success(_success), id(_id) {};
};

class GraphRepository {
public:
	// State.
	virtual bool select(ThoughtId) = 0;
	virtual const State* getState() const = 0;
	// Update operations.
	virtual bool updateThought(ThoughtId, std::string&) = 0;
	virtual CreateResult createThought(
		ThoughtId fromId,
		ConnectionType type,
		bool incoming,
		std::string text
	) = 0;
	virtual bool connectThoughts(
		ThoughtId fromId,
		ThoughtId toId,
		ConnectionType type
	) = 0;
	virtual bool deleteThought(ThoughtId) = 0;
	virtual bool disconnectThoughts(ThoughtId, ThoughtId) = 0;
};

#endif

