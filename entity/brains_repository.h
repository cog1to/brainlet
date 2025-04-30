#ifndef H_BRAINS_REPOSITORY
#define H_BRAINS_REPOSITORY

#include <vector>

#include <QString>

#include "model/model.h"
#include "entity/brain_entity.h"

enum BrainRepositoryError {
	BrainRepositoryErrorNone,
	BrainRepositoryErrorIO,
	BrainRepositoryErrorDuplicate
};

struct ListBrainsResult {
public:
	ListBrainsResult(BrainRepositoryError _err, BrainList _res)
		: error(_err), list(_res) {}
	BrainRepositoryError error;
	BrainList list;
};

struct CreateBrainResult {
public:
	CreateBrainResult(BrainRepositoryError _err, Brain _res)
		: error(_err), result(_res) {}
	BrainRepositoryError error;
	Brain result;	
};

class BrainsRepository {
public:
	virtual ListBrainsResult listBrains() = 0;
	virtual CreateBrainResult createBrain(QString) = 0;
	virtual BrainRepositoryError deleteBrain(QString) = 0;
	virtual BrainRepositoryError renameBrain(QString, QString) = 0;
};

#endif
