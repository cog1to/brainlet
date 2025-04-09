#ifndef H_MEMORY_REPOSITORY
#define H_MEMORY_REPOSITORY

#include <vector>
#include <unordered_map>

#include "model/model.h"
#include "entity/graph_repository.h"
#include "entity/text_repository.h"
#include "entity/search_repository.h"
#include "entity/brains_repository.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/brain_entity.h"

class MemoryRepository:
	public GraphRepository,
	public TextRepository,
	public SearchRepository,
	public BrainsRepository
{
public:
	MemoryRepository(
		std::vector<ThoughtEntity>,
		std::vector<ConnectionEntity>,
		ThoughtId
	);
	~MemoryRepository();
	// GraphRepository.
	bool select(ThoughtId) override;
	const State* getState() const override;
	bool updateThought(ThoughtId, std::string&) override;
	CreateResult createThought(
		ThoughtId fromId,
		ConnectionType type,
		bool incoming,
		std::string text
	) override;
	bool connectThoughts(
		ThoughtId fromId,
		ThoughtId toId,
		ConnectionType type
	) override;
	bool deleteThought(ThoughtId) override;
	bool disconnectThoughts(ThoughtId, ThoughtId) override;
	// TextRepository.
	GetResult getText(ThoughtId) override;
	SaveResult saveText(ThoughtId, std::string) override;
	// SearchRepository.
	SearchResult search(std::string) override;
	// BrainRepository.
	BrainList listBrains() override;
	CreateBrainResult createBrain(std::string) override;
	BrainRepositoryError deleteBrain(std::string) override;
	BrainRepositoryError renameBrain(std::string, std::string) override;

private:
	// List of brains.
	std::vector<BrainEntity> m_brains;
	// Brain.
	std::vector<ThoughtEntity> m_thoughts;
	std::vector<ConnectionEntity> m_connections;
	ThoughtId m_rootId;
	ThoughtId m_currentId;
	State *m_state = nullptr;
	std::unordered_map<ThoughtId, std::string> m_texts;
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

