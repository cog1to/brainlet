#include <vector>

#include <QObject>

#include "presenters/search_presenter.h"
#include "widgets/connection_list_widget.h"

SearchPresenter::SearchPresenter(
	SearchRepository *repo,
	SearchWidget *widget
) : m_repo(repo), m_widget(widget) {
	connect(
		widget, SIGNAL(textChanged(SearchWidget*, QString)),
		this, SLOT(onTextChanged(SearchWidget*, QString))
	);
	connect(
		widget, SIGNAL(thoughtSelected(SearchWidget*, ThoughtId, QString)),
		this, SLOT(onThoughtSelected(SearchWidget*, ThoughtId, QString))
	);
	connect(
		widget,
		SIGNAL(connectionSelected(SearchWidget*, ThoughtId, QString, ConnectionType, bool)),
		this,
		SLOT(onConnectionSelected(SearchWidget*, ThoughtId, QString, ConnectionType, bool))
	);
	connect(
		widget, SIGNAL(searchCanceled(SearchWidget*)),
		this, SLOT(onSearchCanceled(SearchWidget*))
	);
}

void SearchPresenter::onTextChanged(SearchWidget *widget, QString text) {
	if (m_repo == nullptr)
		return;
	if (text.length() < 3) {
		std::vector<ConnectionItem> empty;
		widget->setItems(empty);
		return;
	}

	std::string term = text.toStdString();
	SearchResult result = m_repo->search(term);

	if (result.error != SearchErrorNone) {
		// TODO: show error
	} else {
		std::vector<ConnectionItem> items;

		for (auto it = result.items.begin(); it != result.items.end(); it++) {
			ConnectionItem item = {
				.id = (*it).id,
				.name = QString::fromStdString(*((*it).name))
			};
			items.push_back(item);
		}

		widget->setItems(items);
	}
}

void SearchPresenter::onThoughtSelected(
	SearchWidget*,
	ThoughtId id,
	QString name
) {
	emit searchItemSelected(id, name);
}

void SearchPresenter::onSearchCanceled(SearchWidget*) {
	emit searchCanceled();
}

void SearchPresenter::onConnectionSelected(
	SearchWidget*,
	ThoughtId id,
	QString name,
	ConnectionType type,
	bool incoming
) {
	emit connectionSelected(id, name, type, incoming);
}

// State manipulation.

void SearchPresenter::clear() {
	if (m_widget != nullptr) {
		m_widget->clear();
	}
}

void SearchPresenter::reset() {
	if (m_widget != nullptr) {
		m_widget->reset();
	}
}
