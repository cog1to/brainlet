#ifndef H_TEXT_EDITOR_PRESENTER
#define H_TEXT_EDITOR_PRESENTER

#include <QObject>
#include <QString>

#include "model/thought.h"
#include "entity/text_repository.h"
#include "entity/search_repository.h"
#include "entity/graph_repository.h"
#include "widgets/markdown_edit_widget.h"
#include "widgets/markdown_scroll_widget.h"
#include "presenters/search_presenter.h"
#include "presenters/dismissable_presenter.h"

class TextEditorPresenter: public DismissablePresenter {
	Q_OBJECT

public:
	TextEditorPresenter(
		TextRepository*,
		SearchRepository*,
		MarkdownScrollWidget*
	);
	void setThought(ThoughtId);

signals:
	void textError(MarkdownScrollError);
	void nodeLinkSelected(ThoughtId);
	void connectionCreated();

public slots:
	void onDismiss() override;

private slots:
	void onTextChanged(QString&);
	void onNodeInsertion(QPoint);
	void onSearchCanceled();
	void onConnectionSelected(ThoughtId, QString, ConnectionType, bool);
	void onThoughtSelected(ThoughtId, QString);

private:
	ThoughtId m_id = InvalidThoughtId;
	// Dependencies.
	TextRepository *m_repository = nullptr;
	MarkdownScrollWidget *m_view = nullptr;
	MarkdownEditWidget *m_editView = nullptr;
	// Search.
	SearchRepository *m_searchRepository = nullptr;
	SearchPresenter *m_search = nullptr;
};

#endif

