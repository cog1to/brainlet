#ifndef H_CANVAS_PRESENTER
#define H_CANVAS_PRESENTER

#include <QObject>
#include <QString>

#include "layout/base_layout.h"
#include "entity/graph_repository.h"
#include "entity/search_repository.h"
#include "widgets/canvas_widget.h"

class CanvasPresenter: public QObject {
	Q_OBJECT

public:
	CanvasPresenter(BaseLayout*, GraphRepository*, SearchRepository*, CanvasWidget*);
	void setThought(ThoughtId id);

signals:
	void thoughtSelected(ThoughtId, QString);
	void thoughtRenamed(ThoughtId, QString);

public slots:
	void onThoughtSelected(ThoughtId);

private slots:
	void onThoughtChanged(ThoughtId, QString, std::function<void(bool)>);
	void onThoughtCreated(
		ThoughtId fromId,
		ConnectionType type,
		bool incoming,
		QString text,
		std::function<void(bool, ThoughtId)> callback
	);
	void onThoughtConnected(
		ThoughtId fromId,
		ThoughtId toId,
		ConnectionType type,
		std::function<void(bool)> callback
	);
	void onThoughtDeleted(ThoughtId);
	void onThoughtsDisconnected(ThoughtId, ThoughtId);
	void onShown();
	// Connection suggestions.
	void onNewThoughtTextChanged(QString);

private:
	// State.
	BaseLayout *m_layout;
	GraphRepository *m_repo;
	SearchRepository *m_search;
	CanvasWidget *m_view;
	// Helpers.
	void reloadState();
};

#endif

