#ifndef BRAIN_H
#define BRAIN_H

#include <string>
#include <vector>

class Brain {
public:
	// Constructor and destructor.
	Brain(
		std::string id,
		std::string name,
		uint64_t timestamp
	);
	~Brain();
	// Name.
	const std::string& name() const { return m_name; }
	std::string& name() { return m_name; }
	// Id.
	const std::string id() const { return m_id; }
	const std::string* idPtr() const { return &m_id; }
	// Timestamp.
	const uint64_t timestamp() const { return m_timestamp; }

private:
	std::string m_name;
	std::string m_id;
	uint64_t m_timestamp;
};

#endif

