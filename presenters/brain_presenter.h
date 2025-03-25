#ifndef H_BRAIN_PRESENTER
#define H_BRAIN_PRESENTER

#include <QObject>

#include "model/thought.h"
#include "presenters/canvas_presenter.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/search_presenter.h"
#include "widgets/brain_widget.h"

class BrainPresenter: public QObject {
	Q_OBJECT

public:
	BrainPresenter(
		BrainWidget*,
		CanvasPresenter*,
		TextEditorPresenter*,
		SearchPresenter*
	);

protected slots:
	void onThoughtSelected(ThoughtId, QString);
	void onThoughtRenamed(ThoughtId, QString);
	void onSearchItemSelected(ThoughtId, QString);
	void onThoughtLinkSelected(ThoughtId);

private:
	BrainWidget *m_view;
	CanvasPresenter *m_canvas;
	TextEditorPresenter *m_editor;
	SearchPresenter *m_search;
};

#endif
