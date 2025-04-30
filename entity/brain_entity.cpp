#include <QString>

#include "entity/brain_entity.h"

BrainEntity::BrainEntity(
	QString _id, QString _name, uint64_t _timestamp
) : id(_id), name(_name), timestamp(_timestamp) {};
