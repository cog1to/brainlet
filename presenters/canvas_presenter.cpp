#include "presenters/canvas_presenter.h"

CanvasPresenter::CanvasPresenter(
	BaseLayout *layout,
	Repository *repo,
	BaseCanvasWidget *view
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
		view, SIGNAL(onShown()),
		this, SLOT(onShown())
	);
}

CanvasPresenter::~CanvasPresenter() {
	QObject::disconnect(m_view, nullptr, this, nullptr);
}

// Slots.

void CanvasPresenter::onShown() {
	reloadState();
}

void CanvasPresenter::onThoughtSelected(ThoughtId id) {
	if (m_repo->select(id)) {
		reloadState();
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

// Helpers.

void CanvasPresenter::reloadState() {
	if (auto state = m_repo->getState(); state != nullptr) {
		m_layout->setState(state);
	}
}

