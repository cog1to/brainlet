#ifndef H_MEMORY_REPOSITORY
#define H_MEMORY_REPOSITORY

#include <vector>

#include "model/model.h"
#include "entity/repository.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"

class MemoryRepository: public Repository {
public:
	MemoryRepository(
		std::vector<ThoughtEntity>,
		std::vector<ConnectionEntity>,
		ThoughtId
	);
	~MemoryRepository();
	bool select(ThoughtId) override;
	const State* getState() const override;
	bool updateThought(ThoughtId, std::string&) override;

private:
	std::vector<ThoughtEntity> m_thoughts;
	std::vector<ConnectionEntity> m_connections;
	ThoughtId m_rootId;
	ThoughtId m_currentId;
	State *m_state = nullptr;
	// Helpers.
	void loadState(ThoughtId);
	ThoughtEntity *getThought(ThoughtId);
	std::vector<ConnectionEntity> getParents(ThoughtId);
	std::vector<ConnectionEntity> getChildren(ThoughtId);
	std::vector<ConnectionEntity> getLinks(ThoughtId);
	std::vector<ConnectionEntity> getConnections(
		ThoughtId,
		ConnectionType,
		bool
	);
	bool listContains(std::vector<ThoughtId>&, ThoughtId);
};

#endif

