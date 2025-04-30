#ifndef H_FOLDER_BRAINS_REPOSITORY
#define H_FOLDER_BRAINS_REPOSITORY

#include <QString>
#include <QDir>
#include <QFileInfo>

#include "entity/base_repository.h"
#include "entity/brains_repository.h"
#include "model/model.h"

class FolderBrainsRepository: 
	public BaseRepository, public BrainsRepository
{
public:
	FolderBrainsRepository(QString);
	~FolderBrainsRepository();
	// Brains repository.
	ListBrainsResult listBrains() override;
	CreateBrainResult createBrain(QString) override;
	BrainRepositoryError deleteBrain(QString) override;
	BrainRepositoryError renameBrain(QString, QString) override;

private:
	QString m_base;
	QDir *m_dir = nullptr;
	BrainRepositoryError openOrCreateFolder();
	size_t getSize(QFileInfo);
};

#endif 
