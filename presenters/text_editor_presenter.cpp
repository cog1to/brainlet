#include <QString>

#include "model/model.h"
#include "presenters/text_editor_presenter.h"
#include "widgets/markdown_widget.h"

TextEditorPresenter::TextEditorPresenter(
	TextRepository *repo,
	MarkdownWidget *view
): m_repository(repo), m_view(view) {
	QObject::connect(
		view, SIGNAL(textChanged(QString&)),
		this, SLOT(onTextChanged(QString&))
	);

	QObject::connect(
		this, SIGNAL(textError(MarkdownError)),
		view, SLOT(onError(MarkdownError))
	);
}

TextEditorPresenter::~TextEditorPresenter() {
	QObject::disconnect(m_view, nullptr, this, nullptr);
	QObject::disconnect(this, nullptr, m_view, nullptr);
}

// Loading.

void TextEditorPresenter::setThought(ThoughtId id) {
	m_id = id;
	if (m_view == nullptr)
		return;

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

