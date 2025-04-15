#ifndef H_TABS_PRESENTER
#define H_TABS_PRESENTER

#include <vector>
#include <string>

#include <QObject>
#include <QWidget>

#include "infra/module_factory.h"
#include "infra/dismissable_module.h"
#include "widgets/tabs_widget.h"

struct TabModule {
public:
	TabModule(std::string _id, DismissableModule _mod)
		: id(_id), mod(_mod) {};
	std::string id;
	DismissableModule mod;

};

class TabsPresenter: public QObject {
	Q_OBJECT

public:
	TabsPresenter(TabsWidget*, ModuleFactory*);

private slots:
	void onShown();
	void onBrainSelected(std::string, std::string);
	void onBrainDeleted(std::string);
	void onBrainRenamed(std::string, std::string);
	void onTabClose(int);

private:
	ModuleFactory *m_factory;
	TabsWidget *m_widget;
	std::vector<TabModule> m_tabs;
};

#endif
