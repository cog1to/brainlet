#include <QObject>

#include "model/thought.h"
#include "presenters/canvas_presenter.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/brain_presenter.h"
#include "widgets/brain_widget.h"

BrainPresenter::BrainPresenter(
	BrainWidget *view,
	CanvasPresenter *canvas,
	TextEditorPresenter *editor
)
	: m_view(view), m_canvas(canvas), m_editor(editor)
{
	connect(
		canvas, SIGNAL(thoughtSelected(ThoughtId, QString)),
		this, SLOT(onThoughtSelected(ThoughtId, QString))
	);
	connect(
		canvas, SIGNAL(thoughtRenamed(ThoughtId, QString)),
		this, SLOT(onThoughtRenamed(ThoughtId, QString))
	);
}

void BrainPresenter::onThoughtSelected(ThoughtId id, QString title) {
	if (m_editor != nullptr) {
		m_editor->setThought(id);
	}

	if (m_view != nullptr) {
		m_view->details()->setTitle(title);
	};
}

void BrainPresenter::onThoughtRenamed(ThoughtId id, QString title) {
	if (m_view == nullptr)
		return;

	m_view->details()->setTitle(title);
}

