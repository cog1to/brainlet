#include <vector>
#include <unordered_map>
#include <ctime>

#include <QString>
#include <QRegularExpression>

#include "entity/memory_repository.h"
#include "entity/thought_entity.h"
#include "entity/brain_entity.h"
#include "entity/connection_entity.h"

#include <QDebug>

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
	m_currentId = id;
	loadState(id);
	return true;
}

const State* MemoryRepository::getState() const {
	return m_state;
}

bool MemoryRepository::updateThought(ThoughtId id, std::string& name) {
	if (name.empty())
		return false;

	if (auto thought = getThought(id); thought != nullptr) {
		thought->name = name;
		loadState(m_currentId);
		return true;
	}

	return false;
}

CreateResult MemoryRepository::createThought(
	ThoughtId fromId,
	ConnectionType type,
	bool incoming,
	std::string name
) {
	if (name.empty())
		return CreateResult(false, 0);

	std::time_t result = std::time(nullptr);
	m_thoughts.push_back(ThoughtEntity(result, name));

	if (incoming) {
		m_connections.push_back(ConnectionEntity(result, fromId, type));
	} else {
		m_connections.push_back(ConnectionEntity(fromId, result, type));
	}

	loadState(m_currentId);
	return CreateResult(true, result);
}

bool MemoryRepository::connectThoughts(
	ThoughtId fromId,
	ThoughtId toId,
	ConnectionType type
) {
	bool found = false;

	std::vector<ConnectionEntity>::iterator it;
	for (it = m_connections.begin(); it != m_connections.end(); it++) {
		if (((*it).from == fromId && (*it).to == toId) || ((*it).to == fromId && (*it).from == toId)) {
			(*it).type = type;
			(*it).from = fromId;
			(*it).to = toId;
			found = true;
			break;
		}
	}

	if (!found) {
		m_connections.push_back(ConnectionEntity(fromId, toId, type));
	}

	loadState(m_currentId);
	return true;
}

bool MemoryRepository::deleteThought(ThoughtId id) {
	std::vector<ThoughtEntity> newList;
	for (auto it = m_thoughts.begin(); it != m_thoughts.end(); it++) {
		if ((*it).id == id) continue;
		newList.push_back(*it);
	}

	std::vector<ConnectionEntity> newConns;
	for (auto it = m_connections.begin(); it != m_connections.end(); it++) {
		if (((*it).from == id) || ((*it).to == id)) continue;
		newConns.push_back(*it);
	}

	m_thoughts = newList;
	m_connections = newConns;
	loadState(m_currentId);
	return true;
}

bool MemoryRepository::disconnectThoughts(ThoughtId from, ThoughtId to) {
	std::vector<ConnectionEntity>::iterator it;

	bool found = false;
	for (it = m_connections.begin(); it != m_connections.end();) {
		if (
			((*it).from == from && (*it).to == to) ||
			((*it).from == to && (*it).to == from)
		) {
			it = m_connections.erase(it);
			found = true;
		} else {
			it += 1;
		}
	}

	if (found) {
		loadState(m_currentId);
	}

	return found;
}

// Search.

SearchResult MemoryRepository::search(std::string term) {
	std::vector<SearchItem> result;
	QString qterm = QString::fromStdString(term);

	for (auto it = m_thoughts.begin(); it != m_thoughts.end(); it++) {
		std::string *name = &(*it).name;
		QString qstr = QString::fromStdString(*name);

		if (qstr.contains(qterm, Qt::CaseInsensitive)) {
			result.push_back(SearchItem{
				.id = (*it).id,
				.name	= name
			});
		}
	}

	return SearchResult{
		.error = SearchErrorNone,
		.items = result,
	};
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
	std::vector<ThoughtId>* neighborList[] = { &links, &siblingIds, &children, &parents };
	for (int i = 0; i < 4; i++) {
		auto *list = neighborList[i];
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
				for (auto &c: linkConns) {
					ThoughtId targetId = (c.to == id) ? c.from : c.to;

					bool alreadyLinked = false;
					for (int j = 0; j < i; j++) {
						if (listContains(*neighborList[j], targetId)) {
							alreadyLinked = true;
							break;
						}
					}

					if (!alreadyLinked) {
						nlinks.push_back(targetId);
					}
				}

				thought->links() = nlinks;
			}
		}
	}

	// Construct state.
	State *state = new State(m_rootId, center, siblings);
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

// TextRepository.

GetResult MemoryRepository::getText(ThoughtId id) {
	if (auto found = m_texts.find(id); found != m_texts.end()) {
		GetResult result = GetResult(TextRepositoryNone, found->second);
		return result;
	}

	qDebug() << "Text loaded";
	return GetResult(TextRepositoryNone, "");
}

SaveResult MemoryRepository::saveText(ThoughtId id, std::string text) {
	m_texts.insert_or_assign(id, text);
	qDebug() << "-- Text saved --";
	qDebug() << text;
	return SaveResult(TextRepositoryNone);
}

// BrainRepository

ListBrainsResult MemoryRepository::listBrains() {
	std::vector<Brain> result;
	int size = 0;

	for (auto it = m_brains.begin(); it != m_brains.end(); it++) {
		result.push_back(
			Brain((*it).id, (*it).name, (*it).timestamp)
		);
		size += ((*it).id.length() + (*it).name.length());
	}

	BrainList list = BrainList(
		result,
		size + sizeof(Brain) * result.size(),
		"memory"
	);

	return ListBrainsResult(BrainRepositoryErrorNone, list);
}

CreateBrainResult MemoryRepository::createBrain(std::string name) {
	std::time_t timestamp = std::time(nullptr);

	bool found = false;
	for (auto it = m_brains.begin(); it != m_brains.end(); it++) {
		if ((*it).name == name) {
			found = true;
			break;
		}
	}

	if (found) {
		return CreateBrainResult(
			BrainRepositoryErrorDuplicate,
			Brain("", "", timestamp)
		);
	}

	BrainEntity brain = BrainEntity(name, name, timestamp);
	m_brains.insert(m_brains.begin(), brain);

	CreateBrainResult result = CreateBrainResult(
		BrainRepositoryErrorNone,
		Brain(name, name, timestamp)
	);

	return result;
}

BrainRepositoryError MemoryRepository::renameBrain(
	std::string id,
	std::string name
) {
	bool found = false;
	for (auto it = m_brains.begin(); it != m_brains.end(); it++) {
		if ((*it).name == name) {
			found = true;
			break;
		}
	}

	if (found) {
		return BrainRepositoryErrorDuplicate;
	}

	for (auto it = m_brains.begin(); it != m_brains.end(); it++) {
		if ((*it).id == id) {
			(*it).name = name;
			break;
		}
	}

	return BrainRepositoryErrorNone;
}

BrainRepositoryError MemoryRepository::deleteBrain(std::string id) {
	std::vector<BrainEntity> result;
	for (auto it = m_brains.begin(); it != m_brains.end(); it++) {
		if ((*it).id == id) continue;
		result.push_back(*it);
	}

	m_brains = result;
	return BrainRepositoryErrorNone;
}

// Helpers.

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

// Helpers

std::string MemoryRepository::getBrainName(std::string id) {
	for (auto it = m_brains.begin(); it != m_brains.end(); it++) {
		if ((*it).id == id) {
			return (*it).name;
		}
	}

	return "";
}

