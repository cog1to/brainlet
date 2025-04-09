#ifndef H_BRAINS_REPOSITORY
#define H_BRAINS_REPOSITORY

#include <vector>
#include <string>

#include "model/model.h"
#include "entity/brain_entity.h"

enum BrainRepositoryError {
	BrainRepositoryErrorNone,
	BrainRepositoryErrorIO,
	BrainRepositoryErrorDuplicate
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
	virtual BrainList listBrains() = 0;
	virtual CreateBrainResult createBrain(std::string) = 0;
	virtual BrainRepositoryError deleteBrain(std::string) = 0;
	virtual BrainRepositoryError renameBrain(std::string, std::string) = 0;
};

#endif
