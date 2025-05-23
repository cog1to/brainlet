#include <ctime>

#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QSqlError>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QRegularExpression>

#include <QDebug>

#include "model/thought.h"
#include "entity/database_brain_repository.h"

// Creation.

DatabaseBrainRepository::DatabaseBrainRepository(
	QDir root,
	QSqlDatabase conn
) : m_root(root), m_conn(conn), m_rootId(0), m_currentId(0) {
	select(m_rootId);
}

DatabaseBrainRepository::~DatabaseBrainRepository() {
	if (m_state != nullptr)
		delete m_state;
}

DatabaseBrainRepository *DatabaseBrainRepository::fromDir(QDir root) {
	QSqlDatabase connection;
	bool valid = DatabaseBrainRepository::verify(root, &connection);
	if (!valid)
		return nullptr;

	return new DatabaseBrainRepository(root, connection);
}

bool DatabaseBrainRepository::verify(QDir root, QSqlDatabase *conn) {
	// Check or create root dir.
	if (!root.exists()) {
		qDebug() << "DB: Creating root directory" << root.dirName();
		if (root.mkpath(".") == false)
			return false;
	}

	// Check or create subdirs.
	if (!root.exists("documents")) {
		qDebug() << "DB: Creating documents directory";
		if (root.mkdir("documents") == false)
			return false;
	}

	if (!root.exists("assets")) {
		qDebug() << "DB: Creating assets directory";
		if (root.mkdir("assets") == false)
			return false;
	}

	// Check or create database.
	qDebug() << "DB: Creating database file";
	bool created = DatabaseBrainRepository::createDb(
		QFileInfo(root.dirName()).fileName(),
		QFile(root.filePath("brain.sqlite")),
		conn
	);
	if (!created)
		return false;

	return true;
}

bool DatabaseBrainRepository::createDb(
	QString root,
	QFile file,
	QSqlDatabase *conn
) {
	bool result = false;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(file.fileName());

	qDebug() << "DB: Opening database...";

	result = db.open();
	if (!result)
		return false;

	qDebug() << "DB: Creating tables...";

	// Create tables.
	QSqlQuery thoughtsQuery = QSqlQuery("CREATE TABLE IF NOT EXISTS thoughts (id INTEGER PRIMARY KEY, name TEXT);", db);
	result = thoughtsQuery.exec();
	if (!result)
		return false;

	QSqlQuery connsQuery = QSqlQuery("CREATE TABLE IF NOT EXISTS connections (conn_from INTEGER, conn_to INTEGER, conn_type INTEGER, PRIMARY KEY (conn_from, conn_to));", db);
	result = connsQuery.exec();
	if (!result)
		return false;

	qDebug() << "DB: Creating index...";

	// Create index.
	QSqlQuery indexQuery = QSqlQuery(
		"CREATE INDEX IF NOT EXISTS thought_names ON thoughts (name);",
		db
	);
	result = indexQuery.exec();
	if (!result)
		return false;

	// Check if root thought exists.
	QSqlQuery rootRecordQuery = QSqlQuery(
		"SELECT * FROM thoughts WHERE id == 0;",
		db
	);
	result = rootRecordQuery.exec();
	if (!result)
		return false;

	result = rootRecordQuery.first();
	if (result) {
		qDebug() << "DB: Root exists, exiting...";
		*conn = db;
		return true;
	}

	qDebug() << "DB: Creating root...";

	// Create root thought.
	QString fileName = QFileInfo(file.fileName()).fileName();
	QString rootStr = QString(
		"INSERT INTO thoughts (id, name) VALUES (0, '%1');"
	).arg(root);
	qDebug() << "DB:" << rootStr;

	QSqlQuery rootQuery = QSqlQuery(db);
	rootQuery.prepare(rootStr);
	result = rootQuery.exec();
	if (!result)
		return false;

	*conn = QSqlDatabase(db);
	return true;
}

// Graph interface.

bool DatabaseBrainRepository::select(ThoughtId id) {
	m_currentId = id;
	return loadState(id);
}

const State* DatabaseBrainRepository::getState() const {
	return m_state;
}

bool DatabaseBrainRepository::updateThought(
	ThoughtId id, std::string& name
) {
	bool result = false;
	QString nameStr = QString::fromStdString(name);

	if (name.empty())
		return false;

	// Update the database row.
	QSqlTableModel model = QSqlTableModel(nullptr, m_conn);
	model.setTable("thoughts");
	model.setFilter(QString("id == %1").arg(id));
	model.select();

	if (model.rowCount() == 0) {
		return false;
	}

	// Try to rename the file first.
	QString oldName = model.record(0).value("name").toString();
	QString oldFileName = filePathFromName(oldName, id);
	QString newFileName = filePathFromName(nameStr, id);

	QFile file = QFile(oldFileName);
	if (file.exists()) {
		result = file.rename(newFileName);
		if (!result)
			return false;
	}

	model.setData(model.index(0, 1), nameStr);
	if (!model.submitAll()) {
		// Revert file name change.
		QFile newFile = QFile(newFileName);
		newFile.rename(oldFileName);
		// Return error.
		return false;
	}

	loadState(m_currentId);
	return true;
}

CreateResult DatabaseBrainRepository::createThought(
	ThoughtId fromId,
	ConnectionType type,
	bool incoming,
	std::string text
) {
	bool result = false;
	struct timespec ts;

	timespec_get(&ts, TIME_UTC);
	qlonglong time = ts.tv_sec * 1000000000 + ts.tv_nsec;

	QSqlTableModel model = QSqlTableModel(nullptr, m_conn);
	model.setTable("thoughts");

	// Create a new record.
	QSqlRecord record = model.record();
	record.setValue("id", (qlonglong)time);
	record.setValue("name", QString::fromStdString(text));

	if (!model.insertRecord(-1, record)) {
		qDebug() << "DB error:" << model.lastError().text();
		return CreateResult(false, InvalidThoughtId);
	}

	// Create a connection.
	if (incoming) {
		result = connectThoughts(time, fromId, type);
	} else {
		result = connectThoughts(fromId, time, type);
	}

	if (!result) {
		return CreateResult(false, InvalidThoughtId);
	}

	// Reload state.
	loadState(m_currentId);
	return CreateResult(true, time);
}

bool DatabaseBrainRepository::connectThoughts(
	ThoughtId fromId,
	ThoughtId toId,
	ConnectionType type
) {
	bool result = false;

	result = disconnectThoughts(fromId, toId);
	if (!result)
		return false;

	if (fromId == toId)
		return false;

	QSqlTableModel model = QSqlTableModel(nullptr, m_conn);
	model.setTable("connections");

	QSqlRecord record = model.record();
	record.setValue("conn_from", (qlonglong)fromId);
	record.setValue("conn_to", (qlonglong)toId);
	record.setValue("conn_type", type);

	if (!model.insertRecord(-1, record)) {
		qDebug("DB: Failed to insert connection record");
		return false;
	}

	loadState(m_currentId);
	return true;
}

bool DatabaseBrainRepository::deleteThought(ThoughtId id) {
	bool result = false;
	QSqlQuery query;

	ThoughtEntity thought = getThought(id, &result);
	if (!result) {
		return false;
	}

	// Clear connections.
	query = QSqlQuery(nullptr, m_conn);
	query.prepare(
		"DELETE FROM connections WHERE (conn_from == :f OR conn_to == :f)"
	);
	query.bindValue(":f", (qlonglong)id);

	result = query.exec();
	if (!result)
		return false;

	// Delete the thought from the DB.
	query = QSqlQuery(nullptr, m_conn);
	query.prepare(
		"DELETE FROM thoughts WHERE (id == :tid)"
	);
	query.bindValue(":tid", (qlonglong)id);

	result = query.exec();
	if (!result)
		return false;

	// Delete the text file.
	QString filePath = filePathFromThought(thought);
	QFile file = QFile(filePath);
	if (file.exists()) {
		file.remove();
	}

	loadState(m_currentId);
	return true;
}

bool DatabaseBrainRepository::disconnectThoughts(
	ThoughtId from, ThoughtId to
) {
	bool result = false;
	QSqlQuery query;

	qDebug() << "DB: Deleting connections between" << from << "and" << to;

	// Clear connections.
	query = QSqlQuery(nullptr, m_conn);
	query.prepare("DELETE FROM connections WHERE (conn_from == :f AND conn_to == :t) OR (conn_from == :t AND conn_to == :f)");
	query.bindValue(":f", (qlonglong)from);
	query.bindValue(":t", (qlonglong)to);

	result = query.exec();
	if (!result)
		return false;

	qDebug() << "DB: Deleted connections between" << from << "and" << to;

	loadState(m_currentId);
	return true;
}

// SearchRepository.

SearchResult DatabaseBrainRepository::search(std::string term) {
	int idx;
	std::vector<SearchItem> result;
	QString qterm = QString::fromStdString(term);

	QSqlTableModel model(nullptr, m_conn);
	model.setTable("thoughts");
	model.setFilter(
		QString("name LIKE '%%%1%%'").arg(qterm)
	);

	model.select();

	for (idx = 0; idx < model.rowCount(); idx++) {
		QSqlRecord record = model.record(idx);
		qDebug() << "DB: loading search record" << idx;

		result.push_back(
			SearchItem{
				.id = record.value("id").toULongLong(),
				.name = record.value("name").toString().toStdString(),
			}
		);
	}

	return SearchResult{
		.error = SearchErrorNone,
		.items = result,
	};
}

// TextRepository.

GetResult DatabaseBrainRepository::getText(ThoughtId id) {
	bool result = false;

	ThoughtEntity thought = getThought(id, &result);
	if (!result) {
		return GetResult(TextRepositoryErrorIO, "");
	}

	QString filePath = filePathFromThought(thought);
	QFile file = QFile(filePath);

	if (!file.exists()) {
		return GetResult(TextRepositoryErrorNone, "");
	}

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return GetResult(TextRepositoryErrorIO, "");
	}

	QTextStream in = QTextStream(&file);
	QString content = in.readAll();
	file.close();

	return GetResult(TextRepositoryErrorNone, content);
}

SaveResult DatabaseBrainRepository::saveText(
	ThoughtId id,
	QString text
) {
	bool result = false;

	ThoughtEntity thought = getThought(id, &result);
	if (!result) {
		return SaveResult(TextRepositoryErrorIO);
	}

	QString filePath = filePathFromThought(thought);
	QFile file = QFile(filePath);

	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		return SaveResult(TextRepositoryErrorIO);
	}

	QTextStream out(&file);
	out << text;
	file.close();

	return SaveResult(TextRepositoryErrorNone);
}

// Helpers.

QString DatabaseBrainRepository::filePathFromThought(
	ThoughtEntity& thought
) {
	QString old = QString::fromStdString(thought.name);
	return filePathFromName(old, thought.id);
}

QString DatabaseBrainRepository::filePathFromName(
	QString& name, ThoughtId id
) {
	QString sanitized = QString(name);
	sanitized.replace(
		QRegularExpression("[^\\w\\\"\\']", QRegularExpression::UseUnicodePropertiesOption), "_"
	);

	return m_root.filePath(
		QString("documents/%1_%2.md").arg(sanitized).arg(id)
	);
}

bool DatabaseBrainRepository::loadState(ThoughtId rootId) {
	bool success = false;

	if (m_state != nullptr)
		delete m_state;

	qDebug() << "DB: Reloading state";

	// Find root.
	ThoughtEntity root = getThought(rootId, &success);
	if (!success)
		return false;

	qDebug() << "Thought loaded:" << root.id << root.name;
	qDebug() << "DB: Getting connections";

	std::vector<ConnectionEntity> childConns = getChildren(rootId);
	std::vector<ConnectionEntity> parentConns = getParents(rootId);
	std::vector<ConnectionEntity> linkConns = getLinks(rootId);

	Thought *center = new Thought(
		rootId,
		std::string(root.name),
		parentConns.size() > 0,
		childConns.size() > 0,
		linkConns.size() > 0
	);

	std::unordered_map<ThoughtId, Thought*> *siblings
		= new std::unordered_map<ThoughtId, Thought*>();

	// Children.
	std::vector<ThoughtId> children;
	for (auto& c: childConns) {
		qDebug() << "  child conn:" << c.from << c.to;
		ThoughtEntity entity = getThought(c.to, &success);
		if (!success)
			continue;

		Thought *thought = new Thought(
			entity.id,
			std::string(entity.name),
			getParents(entity.id).size() > 0,
			getChildren(entity.id).size() > 0,
			getLinks(entity.id).size() > 0
		);

		qDebug() << "  created child" << thought->id() << "from" << entity.id;

		children.push_back(entity.id);
		siblings->insert({entity.id, thought});
	}

	center->children() = children;

	// Parents.
	std::vector<ThoughtId> parents;
	for (auto& c: parentConns) {
		qDebug() << "parent conn:" << c.from << c.to;
		ThoughtEntity entity = getThought(c.from, &success);
		if (!success)
			continue;

		Thought *thought = new Thought(
			entity.id,
			std::string(entity.name),
			getParents(entity.id).size() > 0,
			getChildren(entity.id).size() > 0,
			getLinks(entity.id).size() > 0
		);

		parents.push_back(entity.id);
		siblings->insert({entity.id, thought});
	}

	center->parents() = parents;

	// Links.
	std::vector<ThoughtId> links;
	for (auto& c: linkConns) {
		ThoughtId linkId = (c.to == rootId ? c.from : c.to);
		ThoughtEntity entity = getThought(linkId, &success);
		if (!success)
			continue;

		Thought *thought = new Thought(
			entity.id,
			std::string(entity.name),
			getParents(entity.id).size() > 0,
			getChildren(entity.id).size() > 0,
			getLinks(entity.id).size() > 0
		);

		links.push_back(entity.id);
		siblings->insert({entity.id, thought});
	}

	center->links() = links;

	// Siblings.
	std::vector<ThoughtId> siblingIds;
	for (auto& parent: parents) {
		for (auto& c: getChildren(parent)) {
			// Don't load those already loaded as links.
			if (!listContains(links, c.to) && !listContains(parents, c.to)) {
				if (auto found = getThought(c.to, &success); success) {
					// Don't add duplicates if sibling has multiple parents.
					if (!listContains(siblingIds, c.to)) {
						siblingIds.push_back(c.to);

						// Don't overwrite existing nodes.
						if (
							auto existing = siblings->find(found.id);
							existing == siblings->end()
						) {
							// Load thought and add to the map.
							Thought *thought = new Thought(
								found.id,
								found.name,
								getParents(found.id).size() > 0,
								getChildren(found.id).size() > 0,
								getLinks(found.id).size() > 0
							);

							siblings->insert({found.id, thought});
						}
					}
				}
			}
		}
	}

	// Cross-links.
	std::vector<ThoughtId>* neighborList[] = {
		&links, &siblingIds, &children, &parents
	};
	for (int i = 0; i < 4; i++) {
		auto *list = neighborList[i];
		for (auto& id: *list) {
			if (auto node = siblings->find(id); node != siblings->end()) {
				Thought *thought = node->second;

				std::vector<ConnectionEntity> childConns = getChildren(id);
				std::vector<ThoughtId> nchilds;
				for (auto &c: childConns)
					nchilds.push_back(c.to);
				thought->children() = nchilds;

				std::vector<ConnectionEntity> linkConns = getLinks(id);

				std::vector<ThoughtId> nlinks;
				for (auto &c: linkConns) {
					ThoughtId targetId = (c.to == id) ? c.from : c.to;

					bool alreadyLinked = false;
					for (int j = 0; j < i; j++) {
						if (listContains(*neighborList[j], targetId)) {
							alreadyLinked = true;
							break;
						}
					}

					if (!alreadyLinked) {
						nlinks.push_back(targetId);
					}
				}

				thought->links() = nlinks;
			}
		}
	}

	qDebug() << "DB: creating state";

	// Construct state.
	State *state = new State(m_rootId, center, siblings);
	m_state = state;
	return true;
}

ThoughtEntity DatabaseBrainRepository::getThought(
	ThoughtId id,
	bool *success
) {
	QSqlTableModel model(nullptr, m_conn);
	model.setTable("thoughts");
	model.setFilter(QString("id == %1").arg(id));
	model.select();

	if (model.rowCount() > 0) {
		*success = true;
		QSqlRecord rec = model.record(0);
		return ThoughtEntity(
			rec.value("id").toULongLong(),
			rec.value("name").toString().toStdString()
		);
	} else {
		*success = false;
		return ThoughtEntity(InvalidThoughtId, "");
	}
}

std::vector<ConnectionEntity> DatabaseBrainRepository::getParents(
	ThoughtId fromId
) {
	return getConnections(fromId, ConnectionType::child, false);
}

std::vector<ConnectionEntity> DatabaseBrainRepository::getLinks(
	ThoughtId fromId
) {
	auto from = getConnections(fromId, ConnectionType::link, true);
	auto to = getConnections(fromId, ConnectionType::link, false);

	for (auto& node: to)
		from.push_back(node);

	return from;
}

std::vector<ConnectionEntity> DatabaseBrainRepository::getChildren(
	ThoughtId fromId
) {
	return getConnections(fromId, ConnectionType::child, true);
}

std::vector<ConnectionEntity> DatabaseBrainRepository::getConnections(
	ThoughtId id,
	ConnectionType type,
	bool outgoing
) {
	int idx;
	std::vector<ConnectionEntity> result;

	QSqlTableModel model(nullptr, m_conn);
	model.setTable("connections");
	model.setFilter(
		QString(outgoing
			? "(conn_from == %1 AND conn_type == %2)"
			: "(conn_to == %1 AND conn_type == %2)"
		).arg(id).arg(type)
	);
	model.select();

	for (idx = 0; idx < model.rowCount(); idx++) {
		QSqlRecord record = model.record(idx);
		qDebug() << "DB: loading record" << idx;

		result.push_back(
			ConnectionEntity(
				record.value("conn_from").toULongLong(),
				record.value("conn_to").toULongLong(),
				ConnectionType(record.value("conn_type").toInt())
			)
		);
	}

	return result;
}

bool DatabaseBrainRepository::listContains(
	std::vector<ThoughtId>& list,
	ThoughtId id
) {
	for (auto& node: list)
		if (node == id)
			return true;
	return false;
}

