#ifndef BRAIN_LIST_H
#define BRAIN_LIST_H

#include <cstdint>
#include <string>
#include <vector>

#include "model/brain.h"

class BrainList {
public:
	std::vector<Brain> items;
	uint64_t sizeBytes;
	std::string location;
	// Constructor.
	BrainList(std::vector<Brain>, uint64_t size, std::string);
	BrainList();
};

#endif

