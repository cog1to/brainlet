#ifndef H_CANVAS_PRESENTER
#define H_CANVAS_PRESENTER

#include <QObject>
#include <QString>

#include "layout/base_layout.h"
#include "entity/repository.h"
#include "widgets/canvas_widget.h"

class CanvasPresenter: public QObject {
Q_OBJECT

public:
	CanvasPresenter(BaseLayout*, Repository*, CanvasWidget*);
	~CanvasPresenter();

private slots:
	void onThoughtSelected(ThoughtId);
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
		ConnectionType type
	);
	void onThoughtDeleted(ThoughtId);
	void onThoughtsDisconnected(ThoughtId, ThoughtId);
	void onShown();

private:
	// State.
	BaseLayout *m_layout;
	Repository *m_repo;
	CanvasWidget *m_view;
	// Helpers.
	void reloadState();
};

#endif

