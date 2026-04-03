#include <vector>

#include <QString>

#include "model/brain.h"

Brain::Brain(QString id, QString name, uint64_t timestamp, uint64_t size)
	: m_id(id), m_name(name), m_timestamp(timestamp), m_size(size) {}

Brain::~Brain() {}

