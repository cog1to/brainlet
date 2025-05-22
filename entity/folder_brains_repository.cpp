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

		// Get timestamp.
		QDateTime lastModified = file.lastModified();
		uint64_t timestamp = lastModified.toSecsSinceEpoch();

		// Get size.
		size += getSize(file);

		// Save to the list.
		brains.push_back(Brain(name, name, timestamp));
	}

	return ListBrainsResult(
		BrainRepositoryErrorNone,
		BrainList(brains, size, m_dir->absolutePath())
	);
}

CreateBrainResult FolderBrainsRepository::createBrain(
	QString dirName
) {
	assert(m_dir != nullptr);

	// Create a directory for the brain.
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
		Brain(dirName, dirName, timestamp)
	);
}

BrainRepositoryError FolderBrainsRepository::deleteBrain(
	QString name
) {
	assert(m_dir != nullptr);
	qDebug() << "deleting" << name << "from" << m_dir->absolutePath();

	QDir brainDir = QDir(m_dir->absolutePath() + "/" + name);
	if (!brainDir.exists())
		return BrainRepositoryErrorIO;

	bool result = brainDir.removeRecursively();
	if (!result) {
		return BrainRepositoryErrorIO;
	}

	return BrainRepositoryErrorNone;
}

BrainRepositoryError FolderBrainsRepository::renameBrain(
	QString oldName,
	QString newName
) {
	assert(m_dir != nullptr);

	bool result = m_dir->rename(
		oldName,
		newName
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

size_t FolderBrainsRepository::getSize(QFileInfo info) {
	size_t size = 0;

	QDir dir = QDir(info.filePath());
	if (dir.exists() == false) {
		return 0;
	}

	QFileInfoList subfiles = dir.entryInfoList(
		QDir::AllEntries | QDir::NoDotAndDotDot
	);
	for (auto file: subfiles) {
		if (file.isDir())
			size += getSize(file);
		else
			size += file.size();
	}

	return size;
}

