#ifndef H_SEARCH_PRESENTER
#define H_SEARCH_PRESENTER

#include <QObject>
#include <QString>

#include "model/thought.h"
#include "widgets/search_widget.h"
#include "widgets/connection_list_widget.h"
#include "entity/search_repository.h"

class SearchPresenter: public QObject {
	Q_OBJECT

public:
	SearchPresenter(SearchRepository*, SearchWidget*);
	// Removes text and frees focus.
	void clear();
	// Removes text but keeps focus.
	void reset();

signals:
	void searchItemSelected(ThoughtId, QString);
	void connectionSelected(ThoughtId, QString, ConnectionType, bool);
	void searchCanceled();
	void onError(QString);

private slots:
	void onTextChanged(SearchWidget*, QString);
	void onThoughtSelected(SearchWidget*, ThoughtId, QString);
	void onSearchCanceled(SearchWidget*);
	void onConnectionSelected(SearchWidget*, ThoughtId, QString, ConnectionType, bool);

private:
	SearchRepository *m_repo = nullptr;
	SearchWidget *m_widget = nullptr;
};

#endif
