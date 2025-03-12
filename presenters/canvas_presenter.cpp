#include "presenters/canvas_presenter.h"

CanvasPresenter::CanvasPresenter(
	BaseLayout *layout,
	GraphRepository *repo,
	CanvasWidget *view
) : m_layout(layout), m_repo(repo), m_view(view) {
	QObject::connect(
		view, SIGNAL(textChanged(ThoughtId, QString, std::function<void(bool)>)),
		this, SLOT(onThoughtChanged(ThoughtId, QString, std::function<void(bool)>))
	);

	QObject::connect(
		view, SIGNAL(thoughtSelected(ThoughtId)),
		this, SLOT(onThoughtSelected(ThoughtId))
	);

	QObject::connect(
		view, SIGNAL(thoughtCreated(ThoughtId, ConnectionType, bool, QString, std::function<void(bool, ThoughtId)>)),
		this, SLOT(onThoughtCreated(ThoughtId, ConnectionType, bool, QString, std::function<void(bool, ThoughtId)>))
	);

	QObject::connect(
		view, SIGNAL(thoughtConnected(ThoughtId, ThoughtId, ConnectionType)),
		this, SLOT(onThoughtConnected(ThoughtId, ThoughtId, ConnectionType))
	);

	QObject::connect(
		view, SIGNAL(thoughtDeleted(ThoughtId)),
		this, SLOT(onThoughtDeleted(ThoughtId))
	);

	QObject::connect(
		view, SIGNAL(thoughtsDisconnected(ThoughtId, ThoughtId)),
		this, SLOT(onThoughtsDisconnected(ThoughtId, ThoughtId))
	);

	QObject::connect(
		view, SIGNAL(onShown()),
		this, SLOT(onShown())
	);
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
	ConnectionType type
) {
	bool result = m_repo->connect(fromId, toId, type);
	if (result) {
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
	bool result = m_repo->disconnect(from, to);
	if (result) {
		reloadState();
	}
}

// Helpers.

void CanvasPresenter::reloadState() {
	if (auto state = m_repo->getState(); state != nullptr) {
		m_layout->setState(state);
	}
}

