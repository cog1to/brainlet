#include <vector>
#include <unordered_map>

#include <QObject>
#include <QList>

#include "model/state.h"
#include "model/thought.h"
#include "model/connection.h"
#include "widgets/markdown_connections_widget.h"
#include "presenters/connections_presenter.h"

ConnectionsPresenter::ConnectionsPresenter(
	MarkdownConnectionsWidget *view
) : m_view(view) {}

void ConnectionsPresenter::onStateUpdated(const State *state) {
	if (state == nullptr)
		return;

	const Thought *center = state->centralThought();
	if (center == nullptr)
		return;

	const std::unordered_map<ThoughtId, Thought*>* thoughts = state->thoughts();
	if (thoughts == nullptr)
		return;

	QList<Connection> result;
	sortNodes(center->parents(), thoughts, &result, ConnParent);
	sortNodes(center->links(), thoughts, &result, ConnLink);
	sortNodes(center->children(), thoughts, &result, ConnChild);

	m_view->setConnections(result);
}

// Helpers

inline void ConnectionsPresenter::sortNodes(
	// Data to sort nodes.
	const std::vector<ThoughtId>& ids,
	const std::unordered_map<ThoughtId, Thought*>* map,
	// Data to fill connections:
	QList<Connection>* connections,
	// Type of connections.
	ConnectionDirection conn
) {
	for (const auto& id: ids) {
		if (auto found = map->find(id); found != map->end()) {
			QString name = QString::fromStdString(found->second->name());
			connections->push_back(
				Connection(id, name, conn)
			);
		}
	}
}

