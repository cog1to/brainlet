#ifndef H_THOUGHT
#define H_THOUGHT

#include <cstdint>
#include <string>
#include <vector>

typedef uint64_t ThoughtId;
const uint64_t InvalidThoughtId = 0 - 1;

enum ConnectionType { link, child };

class Thought {
public:
	// Constructor and destructor.
	Thought(
		ThoughtId id,
		std::string name,
		bool hasParents = false,
		bool hasChildren = false,
		bool hasLinks = false
	);
	~Thought();
	// Name.
	const std::string& name() const { return m_name; }
	std::string& name() { return m_name; }
	// Id.
	const ThoughtId id() const { return m_id; }
	const ThoughtId* idPtr() const { return &m_id; }
	// Connections.
	bool& hasParents() { return m_conn_up; }
	const bool hasParents() const { return m_conn_up; }
	bool& hasChildren() { return m_conn_down; }
	const bool hasChildren() const { return m_conn_down; }
	bool& hasLinks() { return m_conn_left; }
	const bool hasLinks() const { return m_conn_left; }
	// Node links.
	const std::vector<ThoughtId>& links() const { return m_links; }
	std::vector<ThoughtId>& links() { return m_links; }
	const std::vector<ThoughtId>& parents() const { return m_parents; }
	std::vector<ThoughtId>& parents() { return m_parents; }
	const std::vector<ThoughtId>& children() const { return m_children; }
	std::vector<ThoughtId>& children() { return m_children; }

private:
	std::string m_name;
	ThoughtId m_id;
	bool m_conn_up = false;
	bool m_conn_down = false;
	bool m_conn_left = false;
	std::vector<ThoughtId> m_links;
	std::vector<ThoughtId> m_parents;
	std::vector<ThoughtId> m_children;
};

#endif

