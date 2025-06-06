#ifndef H_MEMORY_REPOSITORY
#define H_MEMORY_REPOSITORY

#include <vector>
#include <unordered_map>

#include <QString>

#include "model/model.h"
#include "entity/base_repository.h"
#include "entity/brain_repository.h"
#include "entity/brains_repository.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/brain_entity.h"

class MemoryRepository:
	public BaseRepository,
	public BrainRepository,
	public BrainsRepository
{
public:
	MemoryRepository(
		std::vector<ThoughtEntity>,
		std::vector<ConnectionEntity>,
		ThoughtId
	);
	~MemoryRepository() override;
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
	SaveResult saveText(ThoughtId, QString) override;
	// SearchRepository.
	SearchResult search(std::string) override;
	// BrainRepository.
	ListBrainsResult listBrains() override;
	CreateBrainResult createBrain(QString) override;
	BrainRepositoryError deleteBrain(QString) override;
	BrainRepositoryError renameBrain(QString, QString) override;
	// Helpers
	QString getBrainName(QString);

private:
	// List of brains.
	std::vector<BrainEntity> m_brains;
	// Brain.
	std::vector<ThoughtEntity> m_thoughts;
	std::vector<ConnectionEntity> m_connections;
	ThoughtId m_rootId;
	ThoughtId m_currentId;
	State *m_state = nullptr;
	std::unordered_map<ThoughtId, QString> m_texts;
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

