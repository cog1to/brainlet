#ifndef BRAIN_LIST_H
#define BRAIN_LIST_H

#include <cstdint>
#include <vector>

#include <QString>

#include "model/brain.h"

class BrainList {
public:
	std::vector<Brain> items;
	uint64_t sizeBytes;
	QString location;
	// Constructor.
	BrainList(std::vector<Brain>, uint64_t size, QString);
	BrainList();
};

#endif

