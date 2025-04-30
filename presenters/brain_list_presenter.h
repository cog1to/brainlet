#ifndef H_BRAIN_LIST_PRESENTER
#define H_BRAIN_LIST_PRESENTER

#include <QObject>
#include <QString>

#include "presenters/dismissable_presenter.h"
#include "widgets/brain_list_widget.h"
#include "entity/brains_repository.h"

class BrainListPresenter: public DismissablePresenter {
	Q_OBJECT

public:
	BrainListPresenter(BrainListWidget*, BrainsRepository*);

signals:
	void brainSelected(QString, QString);
	void brainRenamed(QString, QString);
	void brainDeleted(QString);

private slots:
	void onShown();
	void onBrainSelected(QString, QString);
	void onBrainDeleted(QString);
	void onBrainCreated(QString);
	void onBrainRenamed(QString, QString);
	void onDismiss() override;

private:
	BrainListWidget *m_widget;
	BrainsRepository *m_repo;
	void reload();
};

#endif

