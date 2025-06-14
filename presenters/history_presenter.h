#ifndef H_HISTORY_PRESENTER
#define H_HISTORY_PRESENTER

#include <QString>
#include <QObject>

#include "model/thought.h"
#include "widgets/history_widget.h"

class HistoryPresenter: public QObject {
	Q_OBJECT

public:
	HistoryPresenter(HistoryWidget *view);

signals:
	void itemSelected(ThoughtId, QString&);

public slots:
	void onThoughtSelected(ThoughtId, QString&);

private slots:
	void onItemSelected(ThoughtId, QString&);

private:
	HistoryWidget *m_view;
};

#endif
