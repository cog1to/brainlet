#include <QString>

#include "model/thought.h"
#include "model/connection.h"

Connection::Connection(
	ThoughtId id,
	QString& name,
	ConnectionDirection dir
) : m_id(id), m_name(name), m_dir(dir) {}

ThoughtId Connection::id() {
	return m_id;
}

QString& Connection::name() {
	return m_name;
}

ConnectionDirection Connection::dir() {
	return m_dir;
}

