#include <string>
#include <vector>

#include <QString>
#include <QDir>
#include <QFileInfoList>

#include "entity/folder_brains_repository.h"
#include "entity/brains_repository.h"
#include "model/model.h"

FolderBrainsRepository::FolderBrainsRepository(QString base) {
	m_base = base;
}

FolderBrainsRepository::~FolderBrainsRepository() {
	if (m_dir != nullptr)
		delete m_dir;
}

// Brains repository.

ListBrainsResult FolderBrainsRepository::listBrains() {
	if (auto err = openOrCreateFolder(); err != BrainRepositoryErrorNone) {
		return ListBrainsResult(err, BrainList());
	}

	assert(m_dir != nullptr);

	QFileInfoList list = m_dir->entryInfoList(
		QDir::Dirs | QDir::NoDotAndDotDot
	);

	std::vector<Brain> brains;
	size_t size = 0;
	for (qsizetype i = 0; i < list.size(); ++i) {
		QFileInfo file = list.at(i);

		// Get name.
		QString name = file.fileName();
		std::string stdName = name.toStdString();

		// Get timestamp.
		QDateTime lastModified = file.lastModified();
		uint64_t timestamp = lastModified.toSecsSinceEpoch();

		// Get size.
		size += getSize(file);

		// Save to the list.
		brains.push_back(Brain(stdName, stdName, timestamp));
	}

	return ListBrainsResult(
		BrainRepositoryErrorNone,
		BrainList(brains, size, m_base.toStdString())
	);
}

CreateBrainResult FolderBrainsRepository::createBrain(
	std::string name
) {
	assert(m_dir != nullptr);

	// Create a directory for the brain.
	QString dirName = QString::fromStdString(name);
	if (!m_dir->mkdir(dirName)) {
		return CreateBrainResult(
			BrainRepositoryErrorIO,
			Brain("", "", 0)
		);
	}

	// Create subdirs for thoughts and assets.
	QString docsDirName = dirName + "/" + "documents";
	if (!m_dir->mkdir(docsDirName)) {
		return CreateBrainResult(
			BrainRepositoryErrorIO,
			Brain("", "", 0)
		);
	}

	QString assetsDirName = dirName + "/" + "assets";
	if (!m_dir->mkdir(assetsDirName)) {
		return CreateBrainResult(
			BrainRepositoryErrorIO,
			Brain("", "", 0)
		);
	}

	// Get timestamp.
	QFileInfo file = QFileInfo(m_dir->absoluteFilePath(dirName));
	QDateTime lastModified = file.lastModified();
	uint64_t timestamp = lastModified.toSecsSinceEpoch();

	return CreateBrainResult(
		BrainRepositoryErrorNone,
		Brain(name, name, timestamp)
	);
}

BrainRepositoryError FolderBrainsRepository::deleteBrain(
	std::string name
) {
	assert(m_dir != nullptr);

	bool result = m_dir->remove(
		QString::fromStdString(name)
	);

	if (!result) {
		return BrainRepositoryErrorIO;
	}

	return BrainRepositoryErrorNone;
}

BrainRepositoryError FolderBrainsRepository::renameBrain(
	std::string oldName,
	std::string newName
) {
	assert(m_dir != nullptr);

	bool result = m_dir->rename(
		QString::fromStdString(oldName),
		QString::fromStdString(newName)
	);

	if (!result) {
		return BrainRepositoryErrorIO;
	}

	return BrainRepositoryErrorNone;
}

// Helpers.

BrainRepositoryError FolderBrainsRepository::openOrCreateFolder() {
	if (m_dir != nullptr)
		return BrainRepositoryErrorNone;

	QDir *dir = new QDir(m_base);

	if (dir->exists() == true) {
		m_dir = dir;
		return BrainRepositoryErrorNone;
	}

	if (!dir->mkpath(".")) {
		delete dir;
		return BrainRepositoryErrorIO;
	}

	m_dir = dir;
	return BrainRepositoryErrorNone;
}

size_t FolderBrainsRepository::getSize(QFileInfo dir) {
	// TODO: Recursive file check.
	return dir.size();
}

