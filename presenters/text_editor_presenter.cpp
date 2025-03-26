#include <QString>

#include "model/model.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/search_presenter.h"
#include "widgets/markdown_widget.h"
#include "widgets/search_widget.h"

TextEditorPresenter::TextEditorPresenter(
	TextRepository *repo,
	SearchRepository *search,
	GraphRepository *graph,
	MarkdownWidget *view
)
	: m_repository(repo),
	m_searchRepository(search),
	m_graph(graph),
	m_view(view)
{
	connect(
		view, SIGNAL(textChanged(QString&)),
		this, SLOT(onTextChanged(QString&))
	);

	connect(
		this, SIGNAL(textError(MarkdownError)),
		view, SLOT(onError(MarkdownError))
	);

	connect(
		view, SIGNAL(nodeLinkSelected(ThoughtId)),
		this, SIGNAL(nodeLinkSelected(ThoughtId))
	);

	connect(
		view, SIGNAL(nodeInsertionActivated(QPoint)),
		this, SLOT(onNodeInsertion(QPoint))
	);
}

// Loading.

void TextEditorPresenter::setThought(ThoughtId id) {
	if (m_view == nullptr)
		return;

	// Force save.
	if (m_view->isDirty()) {
		QString text = m_view->text();
		onTextChanged(text);
	}

	// Set ID.
	m_id = id;

	// Empty state.
	if (m_id == InvalidThoughtId) {
		QString empty = QString();
		m_view->load(empty);
		return;
	}

	if (m_repository == nullptr)
		return;

	// Valid state.
	GetResult result = m_repository->getText(m_id);
	if (result.error != TextRepositoryError::TextRepositoryNone) {
		m_id = InvalidThoughtId;
		emit textError(MarkdownError::MarkdownIOError);
		return;
	}

	QString text = QString::fromStdString(result.result);
	m_view->load(text);
}

// Events.

void TextEditorPresenter::onTextChanged(QString& text) {
	if (m_repository == nullptr)
		return;
	if (m_id == InvalidThoughtId)
		return;

	std::string value = text.toStdString();
	SaveResult result = m_repository->saveText(m_id, value);

	if (result.error != TextRepositoryError::TextRepositoryNone) {
		emit textError(MarkdownError::MarkdownIOError);
	}
}

void TextEditorPresenter::onNodeInsertion(QPoint point) {
	if (m_searchRepository == nullptr)
		return;
	if (m_view == nullptr)
		return;

	SearchWidget *widget = new SearchWidget(
		nullptr,
		m_view->style(),
		true,
		tr("Connect to..."),
		true
	);
	SearchPresenter *presenter = new SearchPresenter(m_searchRepository, widget);
	m_search = presenter;

	connect(
		presenter, SIGNAL(searchCanceled()),
		this, SLOT(onSearchCanceled())
	);
	connect(
		presenter, SIGNAL(connectionSelected(ThoughtId, QString, ConnectionType, bool)),
		this, SLOT(onConnectionSelected(ThoughtId, QString, ConnectionType, bool))
	);
	connect(
		presenter, SIGNAL(searchItemSelected(ThoughtId, QString)),
		this, SLOT(onThoughtSelected(ThoughtId, QString))
	);

	m_view->showSearchWidget(widget, point);
}

void TextEditorPresenter::onSearchCanceled() {
	if (m_search != nullptr) {
		delete m_search;
		m_search = nullptr;
	}

	if (m_view != nullptr) {
		m_view->hideSearchWidget();
	}
}

void TextEditorPresenter::onConnectionSelected(
	ThoughtId id,
	QString name,
	ConnectionType type,
	bool incoming
) {
	if (m_graph == nullptr)
		return;

	bool result = false;
	if (incoming) {
		result = m_graph->connect(id, m_id, type);
	} else {
		result = m_graph->connect(m_id, id, type);
	}

	if (result) {
		m_view->hideSearchWidget();
		m_view->insertNodeLink(id, name);
		emit connectionCreated();
	} else {
		m_view->onError(MarkdownIOError);
	}
}

void TextEditorPresenter::onThoughtSelected(ThoughtId id, QString name) {
	m_view->hideSearchWidget();
	m_view->insertNodeLink(id, name);
}

