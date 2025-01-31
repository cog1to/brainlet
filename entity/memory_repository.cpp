#include <vector>
#include <unordered_map>

#include "entity/memory_repository.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"

MemoryRepository::MemoryRepository(
	std::vector<ThoughtEntity> thoughts,
	std::vector<ConnectionEntity> connections,
	ThoughtId root
) : m_thoughts(thoughts), m_connections(connections), m_rootId(root)
{
	select(root);
}

MemoryRepository::~MemoryRepository() {
	if (m_state != nullptr)
		delete m_state;
}

bool MemoryRepository::select(ThoughtId id) {
	loadState(id);
	m_currentId = id;
	return true;
}

const State* MemoryRepository::getState() const {
	return m_state;
}

// Helpers.

void MemoryRepository::loadState(ThoughtId rootId) {
	if (m_state != nullptr)
		delete m_state;

	// Find root.
	ThoughtEntity root(0, "");
	for (auto& t: m_thoughts) {
		if (t.id == rootId)
			root = t;
	}

	std::vector<ConnectionEntity> childConns = getChildren(rootId);
	std::vector<ConnectionEntity> parentConns = getParents(rootId);
	std::vector<ConnectionEntity> linkConns = getLinks(rootId);

	Thought *center = new Thought(
		rootId,
		root.name,
		parentConns.size() > 0,
		childConns.size() > 0,
		linkConns.size() > 0
	);

	std::unordered_map<ThoughtId, Thought*> *siblings = new std::unordered_map<ThoughtId, Thought*>();

	// Children.
	std::vector<ThoughtId> children;
	for (auto& c: childConns) {
		ThoughtEntity *entity = getThought(c.to);
		if (entity == nullptr)
			continue;

		Thought *thought = new Thought(
			entity->id,
			entity->name,
			getParents(entity->id).size() > 0,
			getChildren(entity->id).size() > 0,
			getLinks(entity->id).size() > 0
		);

		children.push_back(entity->id);
		siblings->insert({entity->id, thought});
	}

	center->children() = children;

	// Parents.
	std::vector<ThoughtId> parents;
	for (auto& c: parentConns) {
		ThoughtEntity *entity = getThought(c.from);
		if (entity == nullptr)
			continue;

		Thought *thought = new Thought(
			entity->id,
			entity->name,
			getParents(entity->id).size() > 0,
			getChildren(entity->id).size() > 0,
			getLinks(entity->id).size() > 0
		);

		parents.push_back(entity->id);
		siblings->insert({entity->id, thought});
	}

	center->parents() = parents;

	// Links.
	std::vector<ThoughtId> links;
	for (auto& c: linkConns) {
		ThoughtId linkId = (c.to == rootId ? c.from : c.to);
		ThoughtEntity *entity = getThought(linkId);
		if (entity == nullptr)
			continue;

		Thought *thought = new Thought(
			entity->id,
			entity->name,
			getParents(entity->id).size() > 0,
			getChildren(entity->id).size() > 0,
			getLinks(entity->id).size() > 0
		);

		links.push_back(entity->id);
		siblings->insert({entity->id, thought});
	}

	center->links() = links;

	// Siblings.
	std::vector<ThoughtId> siblingIds;
	for (auto& parent: parents) {
		for (auto& c: getChildren(parent)) {
			// Don't load those already loaded as links.
			if (!listContains(links, c.to)) {
				if (auto found = getThought(c.to); found != nullptr) {
					// Don't add duplicates if sibling has multiple parents.
					if (!listContains(siblingIds, c.to)) {
						siblingIds.push_back(c.to);

						// Don't overwrite existing nodes.
						if (auto existing = siblings->find(found->id); existing == siblings->end()) {
							// Load thought and add to the map.
							Thought *thought = new Thought(
								found->id,
								found->name,
								getParents(found->id).size() > 0,
								getChildren(found->id).size() > 0,
								getLinks(found->id).size() > 0
							);

							siblings->insert({found->id, thought});
						}
					}
				}
			}
		}
	}

	// Cross-links.
	std::vector<ThoughtId>* neighborList[] = { &children, &parents, &links, &siblingIds };
	for (auto *list: neighborList) {
		for (auto& id: *list) {
			if (auto node = siblings->find(id); node != siblings->end()) {
				Thought *thought = node->second;

				std::vector<ConnectionEntity> childConns = getChildren(id);
				std::vector<ThoughtId> nchilds;
				for (auto &c: childConns)
					nchilds.push_back(c.to);
				thought->children() = nchilds;

				std::vector<ConnectionEntity> linkConns = getLinks(id);
				std::vector<ThoughtId> nlinks;
				for (auto &c: linkConns)
					nlinks.push_back((c.to == id) ? c.from : c.to);
				thought->links() = nlinks;
			}
		}
	}

	// Construct state.
	State *state = new State(center, siblings);
	m_state = state;
}

ThoughtEntity *MemoryRepository::getThought(ThoughtId id) {
	std::vector<ThoughtEntity>::iterator it;
	for (it = m_thoughts.begin(); it != m_thoughts.end(); it++) {
		if (it->id == id)
			return &(*it);
	}
	return nullptr;
}

std::vector<ConnectionEntity> MemoryRepository::getParents(
	ThoughtId fromId
) {
	return getConnections(fromId, ConnectionType::child, false);
}

std::vector<ConnectionEntity> MemoryRepository::getLinks(
	ThoughtId fromId
) {
	auto from = getConnections(fromId, ConnectionType::link, true);

	auto to = getConnections(fromId, ConnectionType::link, false);
	for (auto& node: to)
		from.push_back(node);

	return from;
}

std::vector<ConnectionEntity> MemoryRepository::getChildren(
	ThoughtId fromId
) {
	return getConnections(fromId, ConnectionType::child, true);
}

std::vector<ConnectionEntity> MemoryRepository::getConnections(
	ThoughtId id,
	ConnectionType type,
	bool outgoing
) {
	std::vector<ConnectionEntity> result;

	for (auto& c: m_connections) {
		if (c.type == type)
			if ((outgoing && c.from == id) || (!outgoing && c.to == id))
				result.push_back(c);
	}

	return result;
}

bool MemoryRepository::listContains(std::vector<ThoughtId>& list, ThoughtId id) {
	for (auto& node: list)
		if (node == id)
			return true;
	return false;
}

