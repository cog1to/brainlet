#include <QObject>
#include <QString>

#include "presenters/history_presenter.h"
#include "widgets/history_widget.h"

HistoryPresenter::HistoryPresenter(HistoryWidget *view) {
	m_view = view;

	connect(
		view, &HistoryWidget::itemSelected,
		this, &HistoryPresenter::onItemSelected
	);
}

void HistoryPresenter::onThoughtSelected(ThoughtId id, QString& name) {
	m_view->addItem(id, name);
}

void HistoryPresenter::onItemSelected(ThoughtId id, QString& name) {
	emit itemSelected(id, name);
}
