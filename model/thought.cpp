#include <string>

#include "model/thought.h"

Thought::Thought(
	ThoughtId id,
	std::string name,
	bool hasParents,
	bool hasChildren,
	bool hasLinks
)
	: m_name(name)
{
	m_id = id;
	m_conn_up = hasParents;
	m_conn_down = hasChildren;
	m_conn_left = hasLinks;
}

Thought::~Thought() {}

