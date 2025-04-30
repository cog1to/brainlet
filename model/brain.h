#ifndef BRAIN_H
#define BRAIN_H

#include <cstdint>
#include <vector>

#include <QString>

class Brain {
public:
	// Constructor and destructor.
	Brain(
		QString id,
		QString name,
		uint64_t timestamp
	);
	~Brain();
	// Name.
	const QString& name() const { return m_name; }
	QString& name() { return m_name; }
	// Id.
	const QString id() const { return m_id; }
	const QString* idPtr() const { return &m_id; }
	// Timestamp.
	const uint64_t timestamp() const { return m_timestamp; }

private:
	QString m_name;
	QString m_id;
	uint64_t m_timestamp;
};

#endif

