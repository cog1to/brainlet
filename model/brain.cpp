#include <vector>

#include <QString>

#include "model/brain.h"

Brain::Brain(QString id, QString name, uint64_t timestamp)
	: m_id(id), m_name(name), m_timestamp(timestamp) {}

Brain::~Brain() {}

