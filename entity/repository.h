#ifndef H_REPOSITORY
#define H_REPOSITORY

#include "model/model.h"

struct CreateResult {
	bool success;
	ThoughtId id;
};

class Repository {
public:
	// State.
	virtual bool select(ThoughtId) = 0;
	virtual const State* getState() const = 0;
	// Update operations.
	virtual bool updateThought(ThoughtId, std::string&) = 0;
	//virtual bool connect(ThoughtId, ThoughtId, ConnectionType) = 0;
	//virtual CreateResult createThought(ThoughtId, ConnectionType, std::string) = 0;
	//virtual bool deleteThought(ThoughtId) = 0;
};

#endif

