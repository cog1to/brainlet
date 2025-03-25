#include <QString>

#include "model/model.h"
#include "presenters/text_editor_presenter.h"
#include "widgets/markdown_widget.h"

TextEditorPresenter::TextEditorPresenter(
	TextRepository *repo,
	MarkdownWidget *view
)
	: m_repository(repo), m_view(view)
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

