#include <vector>
#include <string>

#include <QObject>
#include <QWidget>

#include "infra/module_factory.h"
#include "infra/dismissable_module.h"
#include "widgets/tabs_widget.h"
#include "presenters/brain_presenter.h"
#include "presenters/tabs_presenter.h"
#include "presenters/brain_list_presenter.h"

TabsPresenter::TabsPresenter(
	TabsWidget *view,
	ModuleFactory *factory
) : m_widget(view), m_factory(factory) {
	connect(
		view, SIGNAL(shown()),
		this, SLOT(onShown())
	);
	connect(
		view, SIGNAL(tabCloseRequested(int)),
		this, SLOT(onTabClose(int))
	);
}

void TabsPresenter::onShown() {
	assert(m_factory != nullptr);

	if (m_tabs.size() > 0)
		return;

	// Make a list module.
	DismissableModule listModule = m_factory->makeBrainsModule();
	assert(listModule.presenter != nullptr);
	assert(listModule.widget != nullptr);

	BrainListPresenter *presenter =
		(BrainListPresenter *)listModule.presenter;

	connect(
		presenter, &BrainListPresenter::brainSelected,
		this,	&TabsPresenter::onBrainSelected
	);
	connect(
		presenter, &BrainListPresenter::brainDeleted,
		this,	&TabsPresenter::onBrainDeleted
	);

	m_tabs.push_back(TabModule("", listModule));

	// Show tab.
	m_widget->addTab(listModule.widget, tr("Welcome"), false);
}

void TabsPresenter::onBrainSelected(std::string id, std::string name) {
	assert(m_factory != nullptr);

	for (auto it = m_tabs.begin(); it != m_tabs.end(); it++) {
		if ((*it).id == id) {
			m_widget->selectWidget((*it).mod.widget);
			return;
		}
	}

	DismissableModule brainModule = m_factory->makeBrainModule(id);
	assert(brainModule.presenter != nullptr);
	assert(brainModule.widget != nullptr);

	m_tabs.push_back(TabModule(id, brainModule));
	m_widget->addTab(brainModule.widget, QString::fromStdString(name), true);
}

void TabsPresenter::onBrainDeleted(std::string id) {
	for (auto it = m_tabs.begin(); it != m_tabs.end(); it++) {
		if ((*it).id == id) {
			m_widget->deleteWidget((*it).mod.widget);
			m_tabs.erase(it);
			return;
		}
	}
}

void TabsPresenter::onTabClose(int idx) {
	assert(idx < m_tabs.size());

	DismissableModule mod = m_tabs[idx].mod;
	mod.presenter->onDismiss();

	m_widget->removeTab(idx);
	delete mod.presenter;
	delete mod.widget;
	delete mod.repo;

	m_tabs.erase(m_tabs.begin() + idx);
}

