#ifndef H_CONNECTION_MODEL
#define H_CONNECTION_MODEL

#include <QString>

#include "model/thought.h"

enum ConnectionDirection {
	ConnParent,
	ConnChild,
	ConnLink
};

class Connection {
public:
	Connection(ThoughtId, QString&, ConnectionDirection);
	ThoughtId id();
	QString& name();
	ConnectionDirection dir();

private:
	ThoughtId m_id;
	QString m_name;
	ConnectionDirection m_dir;
};

#endif

