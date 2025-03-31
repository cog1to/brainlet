#include <string>
#include <vector>

#include "model/brain.h"

Brain::Brain(std::string id, std::string name, uint64_t timestamp)
	: m_id(id), m_name(name), m_timestamp(timestamp) {}

Brain::~Brain() {}

