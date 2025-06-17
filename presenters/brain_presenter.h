#ifndef H_BRAIN_PRESENTER
#define H_BRAIN_PRESENTER

#include <QObject>

#include "model/thought.h"
#include "presenters/canvas_presenter.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/search_presenter.h"
#include "presenters/dismissable_presenter.h"
#include "presenters/history_presenter.h"
#include "presenters/connections_presenter.h"
#include "widgets/brain_widget.h"

class BrainPresenter: public DismissablePresenter {
	Q_OBJECT

public:
	BrainPresenter(
		BrainWidget*,
		CanvasPresenter*,
		TextEditorPresenter*,
		SearchPresenter*,
		HistoryPresenter*,
		ConnectionsPresenter*
	);
	~BrainPresenter();

protected slots:
	void onThoughtSelected(ThoughtId, QString);
	void onThoughtRenamed(ThoughtId, QString);
	void onSearchItemSelected(ThoughtId, QString);
	void onThoughtLinkSelected(ThoughtId);
	void onConnectionCreated();
	void onItemSelected(ThoughtId, QString&);
	void onDismiss() override;

private:
	BrainWidget *m_view;
	CanvasPresenter *m_canvas;
	TextEditorPresenter *m_editor;
	SearchPresenter *m_search;
	HistoryPresenter *m_history;
	ConnectionsPresenter *m_conns;
};

#endif
