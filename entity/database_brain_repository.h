#include <string>

#include <QDir>
#include <QFile>
#include <QString>
#include <QSqlDatabase>

#include "model/model.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/graph_repository.h"
#include "entity/search_repository.h"
#include "entity/text_repository.h"

class DatabaseBrainRepository
	: public GraphRepository, public SearchRepository, public TextRepository
{
public:
	// Constructor.
	static DatabaseBrainRepository *fromDir(QDir);
	~DatabaseBrainRepository();
	// Graph Repository.
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
	// Search repository.
	SearchResult search(std::string) override;
	// Text repository.
	GetResult getText(ThoughtId) override;
	SaveResult saveText(ThoughtId, QString) override;

protected:
	DatabaseBrainRepository(QDir, QSqlDatabase);
	static bool verify(QDir, QSqlDatabase*);
	static bool createDb(QString, QFile, QSqlDatabase*);
	// Helpers.
	bool listContains(
		std::vector<ThoughtId>&,
		ThoughtId
	);
	std::vector<ConnectionEntity> getConnections(
		ThoughtId,
		ConnectionType,
		bool
	);
	std::vector<ConnectionEntity> getChildren(ThoughtId);
	std::vector<ConnectionEntity> getLinks(ThoughtId);
	std::vector<ConnectionEntity> getParents(ThoughtId);
	ThoughtEntity getThought(ThoughtId, bool*);
	bool loadState(ThoughtId);
	QString filePathFromThought(ThoughtEntity&);
	QString filePathFromName(QString&, ThoughtId id);

private:
	// Database.
	QDir m_root;
	QSqlDatabase m_conn;
	// State.
	State *m_state = nullptr;
	ThoughtId m_rootId;
	ThoughtId m_currentId;
};

