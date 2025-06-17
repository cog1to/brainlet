#include <QObject>

#include "model/thought.h"
#include "presenters/canvas_presenter.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/brain_presenter.h"
#include "presenters/search_presenter.h"
#include "presenters/connections_presenter.h"
#include "widgets/brain_widget.h"

BrainPresenter::BrainPresenter(
	BrainWidget *view,
	CanvasPresenter *canvas,
	TextEditorPresenter *editor,
	SearchPresenter *search,
	HistoryPresenter *history,
	ConnectionsPresenter* conns
)
	: m_view(view), m_canvas(canvas), m_editor(editor), m_search(search),
	m_history(history), m_conns(conns)
{
	if (canvas != nullptr) {
		connect(
			canvas, SIGNAL(thoughtSelected(ThoughtId, QString)),
			this, SLOT(onThoughtSelected(ThoughtId, QString))
		);
		connect(
			canvas, SIGNAL(thoughtRenamed(ThoughtId, QString)),
			this, SLOT(onThoughtRenamed(ThoughtId, QString))
		);
	}

	if (search != nullptr) {
		connect(
			search, SIGNAL(searchItemSelected(ThoughtId, QString)),
			this, SLOT(onSearchItemSelected(ThoughtId, QString))
		);
	}

	if (editor != nullptr) {
		connect(
			editor, SIGNAL(nodeLinkSelected(ThoughtId)),
			this,	SLOT(onThoughtLinkSelected(ThoughtId))
		);
	}

	if (editor != nullptr && canvas != nullptr) {
		connect(
			editor, SIGNAL(connectionCreated()),
			canvas,	SLOT(reload())
		);
	}

	if (history != nullptr) {
		connect(
			history, SIGNAL(itemSelected(ThoughtId, QString&)),
			this, SLOT(onItemSelected(ThoughtId, QString&))
		);
	}

	if (conns != nullptr) {
		connect(
			conns, &ConnectionsPresenter::nodeLinkSelected,
			this, &BrainPresenter::onThoughtLinkSelected
		);
	}

	if (conns != nullptr && canvas != nullptr) {
		connect(
			canvas, SIGNAL(stateUpdated(const State*)),
			conns, SLOT(onStateUpdated(const State*))
		);
	}
}

BrainPresenter::~BrainPresenter() {
	if (m_canvas != nullptr)
		delete m_canvas;
	if (m_editor != nullptr)
		delete m_editor;
	if (m_search != nullptr)
		delete m_search;
	if (m_history != nullptr)
		delete m_history;
	if (m_conns != nullptr)
		delete m_conns;
}

void BrainPresenter::onThoughtSelected(ThoughtId id, QString title) {
	if (m_editor != nullptr) {
		m_editor->setThought(id);
	}

	if (m_view != nullptr) {
		m_view->details()->setTitle(title);
	};

	if (m_search != nullptr) {
		m_search->clear();
	}

	if (m_history != nullptr) {
		m_history->onThoughtSelected(id, title);
	}
}

void BrainPresenter::onItemSelected(ThoughtId id, QString& title) {
	m_canvas->setThought(id);
	onThoughtSelected(id, title);
}

void BrainPresenter::onThoughtRenamed(ThoughtId id, QString title) {
	if (m_view == nullptr)
		return;

	m_view->details()->setTitle(title);
}

void BrainPresenter::onSearchItemSelected(ThoughtId id, QString title) {
	if (m_canvas != nullptr) {
		m_canvas->setThought(id);
	};

	if (m_editor != nullptr) {
		m_editor->setThought(id);
	}

	if (m_view != nullptr) {
		m_view->details()->setTitle(title);
	};

	if (m_search != nullptr) {
		m_search->reset();
	}
}

void BrainPresenter::onThoughtLinkSelected(ThoughtId id) {
	m_canvas->onThoughtSelected(id);
}

void BrainPresenter::onConnectionCreated() {
	if (m_canvas != nullptr) {
		m_canvas->reload();
	}
}

void BrainPresenter::onDismiss() {
	// Force data save on editor.
	if (m_editor != nullptr) {
		m_editor->onDismiss();
	}
}

