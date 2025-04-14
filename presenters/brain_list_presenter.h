#ifndef H_BRAIN_LIST_PRESENTER
#define H_BRAIN_LIST_PRESENTER

#include <string>

#include <QObject>

#include "presenters/dismissable_presenter.h"
#include "widgets/brain_list_widget.h"
#include "entity/brains_repository.h"

class BrainListPresenter: public DismissablePresenter {
	Q_OBJECT

public:
	BrainListPresenter(BrainListWidget*, BrainsRepository*);

signals:
	void brainSelected(std::string, std::string);
	void brainDeleted(std::string);

private slots:
	void onShown();
	void onBrainSelected(std::string, std::string);
	void onBrainDeleted(std::string);
	void onBrainCreated(std::string);
	void onBrainRenamed(std::string, std::string);
	void onDismiss() override;

private:
	BrainListWidget *m_widget;
	BrainsRepository *m_repo;
};

#endif

