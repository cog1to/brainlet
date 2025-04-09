#ifndef H_BRAIN_LIST_PRESENTER
#define H_BRAIN_LIST_PRESENTER

#include <string>

#include <QObject>

#include "widgets/brain_list_widget.h"
#include "entity/brains_repository.h"

class BrainListPresenter: public QObject {
	Q_OBJECT

public:
	BrainListPresenter(BrainListWidget*, BrainsRepository*);

signals:
	void brainSelected(std::string);

private slots:
	void onShown();
	void onBrainSelected(std::string);
	void onBrainDeleted(std::string);
	void onBrainCreated(std::string);
	void onBrainRenamed(std::string, std::string);

private:
	BrainListWidget *m_widget;
	BrainsRepository *m_repo;
};

#endif

