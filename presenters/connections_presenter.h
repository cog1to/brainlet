#ifndef H_CONNECTIONS_PRESENTER
#define H_CONNECTIONS_PRESENTER

#include <vector>
#include <unordered_map>

#include <QObject>
#include <QList>

#include "model/state.h"
#include "model/thought.h"
#include "model/connection.h"
#include "widgets/markdown_connections_widget.h"

class ConnectionsPresenter: public QObject {
	Q_OBJECT

public:
	ConnectionsPresenter(MarkdownConnectionsWidget*);

public slots:
	void onStateUpdated(const State*);

private:
	MarkdownConnectionsWidget *m_view;
	// Helpers
	inline void sortNodes(
		// Data to sort nodes.
		const std::vector<ThoughtId>& ids,
		const std::unordered_map<ThoughtId, Thought*>* map,
		// Data to fill connections:
		QList<Connection>* connections,
		// Type of connections.
		ConnectionDirection conn
	);
};

#endif

