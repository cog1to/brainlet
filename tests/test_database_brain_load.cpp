#include <QDir>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <QDebug>

#include "entity/database_brain_repository.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);
	QDir dir = QDir("test_brain");
	if (dir.exists()) {
		dir.removeRecursively();
	}

	DatabaseBrainRepository *repo = DatabaseBrainRepository::fromDir(dir);
	if (repo == nullptr) {
		qDebug("Failed");
		return 1;
	} else {
		qDebug("Succeeded");
	}

	CreateResult res = repo->createThought(0, ConnectionType::link, false, "link 1");
	if (res.success) {
		qDebug("Created");
	} else {
		qDebug("Failed to create");
		return 1;
	}

	res = repo->createThought(0, ConnectionType::child, true, "parent 1");
	if (res.success) {
		qDebug("Created parent");
	} else {
		qDebug("Failed to create parent");
		return 1;
	}

	std::string newName = "Long parent name 2";
	bool updateRes = repo->updateThought(res.id, newName);
	if (!updateRes) {
		qDebug("Failed to update");
		return 1;
	}

	bool reconnRes = repo->connectThoughts(0, res.id, ConnectionType::child);
	if (!updateRes) {
		qDebug("Failed to reconnect");
		return 1;
	}

	bool deleteRes = repo->deleteThought(res.id);
	if (!deleteRes) {
		qDebug("Failed to delete");
		return 1;
	}

	delete repo;
	return 0;
}
