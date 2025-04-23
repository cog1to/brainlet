#ifndef H_FOLDER_BRAINS_REPOSITORY
#define H_FOLDER_BRAINS_REPOSITORY

#include <string>

#include <QDir>
#include <QFileInfo>

#include "entity/brains_repository.h"
#include "model/model.h"

class FolderBrainsRepository: public BrainsRepository {
public:
	FolderBrainsRepository(std::string);
	~FolderBrainsRepository();
	// Brains repository.
	ListBrainsResult listBrains() override;
	CreateBrainResult createBrain(std::string) override;
	BrainRepositoryError deleteBrain(std::string) override;
	BrainRepositoryError renameBrain(std::string, std::string) override;

private:
	std::string m_base;
	QDir *m_dir = nullptr;
	BrainRepositoryError openOrCreateFolder();
	size_t getSize(QFileInfo);
};

#endif 
