#include <string>
#include <cassert>
#include <iostream>

#include "entity/memory_repository.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"

int main(int argc, char *argv[]) {
	std::vector<ThoughtEntity> thoughts = {
		ThoughtEntity(0, "Brain"),
		ThoughtEntity(1, "Parent 1"),
		ThoughtEntity(2, "Parent 2"),
		ThoughtEntity(3, "Sibling"),
		ThoughtEntity(4, "Link 1"),
		ThoughtEntity(5, "Link 2")
	};
	std::vector<ConnectionEntity> conns = {
		ConnectionEntity(0, 4, ConnectionType::link),
		ConnectionEntity(0, 5, ConnectionType::link),
		ConnectionEntity(1, 0, ConnectionType::child),
		ConnectionEntity(1, 3, ConnectionType::child),
		ConnectionEntity(1, 4, ConnectionType::child),
		ConnectionEntity(2, 0, ConnectionType::child),
		ConnectionEntity(2, 5, ConnectionType::link)
	};
	ThoughtId root = 0;

	MemoryRepository repo = MemoryRepository(thoughts, conns, root);
	
	// 1. Check default state.
	const State *state = repo.getState();
	const Thought *center = state->centralThought();
	assert(center->name() == "Brain");
	assert(center->id() == 0);

	// Links.
	ThoughtId bLinks[] = {4, 5};
	for (ThoughtId id: bLinks) {
		bool found = false;
		for (auto link: center->links()) {
			if (link == id) {
				found = true; break;
			}
		}

		assert(found);
	}

	// Parents.
	ThoughtId bParents[] = {1, 2};
	for (ThoughtId id: bParents) {
		bool found = false;
		for (auto link: center->parents()) {
			if (link == id) {
				found = true; break;
			}
		}

		assert(found);
	}

	// Parent 1.
	auto it = state->thoughts()->find(1);
	assert(it != state->thoughts()->end());
	assert(it->second->children().size() == 3);
	for (auto childId: it->second->children()) {
		assert(childId == 0 || childId == 4 || childId == 3);
	}

	// Parent 2.
	it = state->thoughts()->find(2);
	assert(it != state->thoughts()->end());
	assert(it->second->children()[0] == 0);
	assert(it->second->links()[0] == 5);
	
	// Select Link 1.
	repo.select(4);
	state = repo.getState();

	center = state->centralThought();
	assert(center->name() == "Link 1");
	assert(center->parents()[0] == 1);
	assert(center->links()[0] == 0);
}
