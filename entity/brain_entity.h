#ifndef H_BRAIN_ENTITY
#define H_BRAIN_ENTITY

#include <QString>

class BrainEntity {
public:
	BrainEntity(QString, QString, uint64_t);
	QString id;
	QString name;
	uint64_t timestamp;
};

#endif

