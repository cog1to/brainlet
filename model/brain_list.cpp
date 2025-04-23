#include <cstdint>
#include <string>
#include <vector>

#include "model/brain.h"
#include "model/brain_list.h"

BrainList::BrainList(
	std::vector<Brain> _items,
	uint64_t _size,
	std::string _location
) : items(_items), sizeBytes(_size), location(_location) {}

BrainList::BrainList() {
	items = std::vector<Brain>();
	sizeBytes = 0;
	location = "";
}
