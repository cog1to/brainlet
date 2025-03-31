#include <QObject>

#include "presenters/canvas_presenter.h"

CanvasPresenter::CanvasPresenter(
	BaseLayout *layout,
	GraphRepository *repo,
	SearchRepository *search,
	CanvasWidget *view
) : m_layout(layout), m_repo(repo), m_search(search), m_view(view) {
	connect(
		view, SIGNAL(textChanged(ThoughtId, QString, std::function<void(bool)>)),
		this, SLOT(onThoughtChanged(ThoughtId, QString, std::function<void(bool)>))
	);

	connect(
		view, SIGNAL(thoughtSelected(ThoughtId)),
		this, SLOT(onThoughtSelected(ThoughtId))
	);

	connect(
		view,SIGNAL(thoughtCreated(ThoughtId, ConnectionType, bool, QString, std::function<void(bool, ThoughtId)>)),
		this, SLOT(onThoughtCreated(ThoughtId, ConnectionType, bool, QString, std::function<void(bool, ThoughtId)>))
	);

	connect(
		view, SIGNAL(thoughtConnected(ThoughtId, ThoughtId, ConnectionType, std::function<void(bool)>)),
		this, SLOT(onThoughtConnected(ThoughtId, ThoughtId, ConnectionType, std::function<void(bool)>))
	);

	connect(
		view, SIGNAL(thoughtDeleted(ThoughtId)),
		this, SLOT(onThoughtDeleted(ThoughtId))
	);

	connect(
		view, SIGNAL(thoughtsDisconnected(ThoughtId, ThoughtId)),
		this, SLOT(onThoughtsDisconnected(ThoughtId, ThoughtId))
	);

	connect(
		view, SIGNAL(shown()),
		this, SLOT(onShown())
	);

	connect(
		view, SIGNAL(newThoughtTextChanged(QString)),
		this, SLOT(onNewThoughtTextChanged(QString))	
	);
}

void CanvasPresenter::setThought(ThoughtId id) {
	if (m_repo->select(id)) {
		reloadState();
	}
}

// Slots.

void CanvasPresenter::onShown() {
	reloadState();

	// Notify other widgets.
	if (auto state = m_repo->getState(); state != nullptr) {
		if (const Thought* center = state->centralThought(); center != nullptr) {
			emit thoughtSelected(center->id(), QString::fromStdString(center->name()));
		}
	}
}

void CanvasPresenter::onThoughtSelected(ThoughtId id) {
	if (m_repo->select(id)) {
		reloadState();

		if (auto state = m_repo->getState(); state != nullptr) {
			if (const Thought* center = state->centralThought(); center != nullptr) {
				emit thoughtSelected(center->id(), QString::fromStdString(center->name()));
			}
		}
	}
}

void CanvasPresenter::onThoughtChanged(
	ThoughtId id,
	QString text,
	std::function<void(bool)> callback
) {
	std::string value = text.toStdString();
	bool result = m_repo->updateThought(id, value);
	callback(result);

	if (result) {
		reloadState();
		emit thoughtRenamed(id, text);
	}
}

void CanvasPresenter::onThoughtCreated(
	ThoughtId fromId,
	ConnectionType connection,
	bool incoming,
	QString text,
	std::function<void(bool, ThoughtId)> callback
) {
	std::string value = text.toStdString();
	CreateResult result = m_repo->createThought(fromId, connection, incoming, value);
	callback(result.success, result.id);

	if (result.success) {
		reloadState();
	}
}

void CanvasPresenter::onThoughtConnected(
	ThoughtId fromId,
	ThoughtId toId,
	ConnectionType type,
	std::function<void(bool)> callback
) {
	bool result = m_repo->connectThoughts(fromId, toId, type);
	callback(result);

	if (result) {
		if (m_view != nullptr)
			m_view->hideSuggestions();
		reloadState();
	}
}

void CanvasPresenter::onThoughtDeleted(ThoughtId id) {
	bool result = m_repo->deleteThought(id);
	if (result) {
		reloadState();
	}
}

void CanvasPresenter::onThoughtsDisconnected(ThoughtId from, ThoughtId to) {
	bool result = m_repo->disconnectThoughts(from, to);
	if (result) {
		reloadState();
	}
}

void CanvasPresenter::onNewThoughtTextChanged(QString text) {
	if (m_view == nullptr)
		return;
	if (m_search == nullptr)
		return;

	if (text.length() < 3) {
		m_view->hideSuggestions();
		return;
	}

	const State *state = m_repo->getState();
	if (state == nullptr)
		return;
	const std::unordered_map<ThoughtId, Thought*>* thoughts = state->thoughts();

	std::string term = text.toStdString();
	SearchResult result = m_search->search(term);

	if (result.error != SearchErrorNone) {
		// TODO: show error
	} else {
		std::vector<ConnectionItem> items;

		for (auto it = result.items.begin(); it != result.items.end(); it++) {
			// Don't show currently visible items in suggestions.
			// Can potentially change later if we'll make "full graph" layout.
			if (auto found = thoughts->find((*it).id); found != thoughts->end()) {
				continue;
			}

			ConnectionItem item = {
				.id = (*it).id,
				.name = QString::fromStdString(*((*it).name))
			};
			items.push_back(item);
		}

		m_view->showSuggestions(items);
	}
}

void CanvasPresenter::reload() {
	reloadState();
}

// Helpers.

void CanvasPresenter::reloadState() {
	if (auto state = m_repo->getState(); state != nullptr) {
		m_layout->setState(state);
	}
}

