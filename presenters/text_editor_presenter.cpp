#include <QString>

#include "model/model.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/search_presenter.h"
#include "widgets/markdown_widget.h"
#include "widgets/search_widget.h"

TextEditorPresenter::TextEditorPresenter(
	TextRepository *repo,
	SearchRepository *search,
	MarkdownScrollWidget *view
)
	: m_repository(repo),
	m_searchRepository(search),
	m_view(view)
{
	m_editView = view->markdownWidget();

	connect(
		m_editView, SIGNAL(textChanged(QString&)),
		this, SLOT(onTextChanged(QString&))
	);

	connect(
		this, SIGNAL(textError(MarkdownScrollError)),
		view, SLOT(onError(MarkdownScrollError))
	);

	connect(
		m_editView, SIGNAL(nodeLinkSelected(ThoughtId)),
		this, SIGNAL(nodeLinkSelected(ThoughtId))
	);

	connect(
		m_editView, SIGNAL(nodeInsertionActivated(QPoint)),
		this, SLOT(onNodeInsertion(QPoint))
	);
}

// Loading.

void TextEditorPresenter::setThought(ThoughtId id) {
	if (m_view == nullptr)
		return;

	// Force save.
	if (m_editView->isDirty()) {
		QString text = m_editView->text();
		onTextChanged(text);
	}

	// Set ID.
	m_id = id;

	// Empty state.
	if (m_id == InvalidThoughtId) {
		QString empty = QString();
		m_editView->load(empty);
		return;
	}

	if (m_repository == nullptr)
		return;

	// Valid state.
	GetResult result = m_repository->getText(m_id);
	if (result.error != TextRepositoryError::TextRepositoryErrorNone) {
		m_id = InvalidThoughtId;
		emit textError(MarkdownScrollError::MarkdownScrollIOError);
		return;
	}

	QString text = result.result;
	qDebug() << "loaded" << text;
	m_editView->load(text);
}

// Events.

void TextEditorPresenter::onTextChanged(QString& text) {
	if (m_repository == nullptr)
		return;
	if (m_id == InvalidThoughtId)
		return;

	SaveResult result = m_repository->saveText(m_id, text);

	if (result.error != TextRepositoryError::TextRepositoryErrorNone) {
		emit textError(MarkdownScrollError::MarkdownScrollIOError);
	}
}

void TextEditorPresenter::onNodeInsertion(QPoint point) {
	if (m_searchRepository == nullptr)
		return;
	if (m_editView == nullptr)
		return;

	SearchWidget *widget = new SearchWidget(
		nullptr,
		m_editView->style(),
		true,
		tr("Connect to..."),
		true
	);
	SearchPresenter *presenter = new SearchPresenter(
		m_searchRepository,
		widget
	);
	m_search = presenter;

	connect(
		presenter, SIGNAL(searchCanceled()),
		this, SLOT(onSearchCanceled())
	);
	connect(
		presenter, &SearchPresenter::connectionSelected,
		this, &TextEditorPresenter::onConnectionSelected
	);
	connect(
		presenter, SIGNAL(searchItemSelected(ThoughtId, QString)),
		this, SLOT(onThoughtSelected(ThoughtId, QString))
	);

	m_editView->showSearchWidget(widget, point);
}

void TextEditorPresenter::onSearchCanceled() {
	if (m_search != nullptr) {
		delete m_search;
		m_search = nullptr;
	}

	if (m_editView != nullptr) {
		m_editView->hideSearchWidget();
	}
}

void TextEditorPresenter::onConnectionSelected(
	ThoughtId id,
	QString name,
	ConnectionType type,
	bool incoming
) {
	bool result = false;

	if (m_repository == nullptr)
		return;

	if (incoming) {
		result = m_repository->connectThoughts(id, m_id, type);
	} else {
		result = m_repository->connectThoughts(m_id, id, type);
	}

	if (result) {
		m_editView->hideSearchWidget();
		m_editView->insertNodeLink(id, name);
		m_editView->setFocus();
		emit connectionCreated();
	} else {
		m_view->onError(MarkdownScrollIOError);
	}
}

void TextEditorPresenter::onThoughtSelected(
	ThoughtId id,
	QString name
) {
	if (m_editView == nullptr)
		return;

	m_editView->hideSearchWidget();
	m_editView->insertNodeLink(id, name);
	m_editView->setFocus();
}

void TextEditorPresenter::onDismiss() {
	if (m_editView == nullptr)
		return;

	if (m_editView->isDirty()) {
		QString text = m_editView->text();
		onTextChanged(text);
	}
}

