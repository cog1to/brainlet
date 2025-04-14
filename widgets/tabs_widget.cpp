#include <vector>
#include <string>

#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QShowEvent>
#include <QBoxLayout>
#include <QTabBar>

#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/tabs_widget.h"

TabsWidget::TabsWidget(
	QWidget *parent,
	Style *style
)
	: BaseWidget(parent, style),
	m_layout(this)
{
	m_tabWidget = new QTabWidget(nullptr);
	m_tabWidget->setTabsClosable(true);
	// TODO: Customize/subclass QTabBar. Looks out of place on Mac.

	m_layout.addWidget(m_tabWidget);

	connect(
		m_tabWidget, &QTabWidget::tabCloseRequested,
		this, &TabsWidget::onTabClose		
	);
}

void TabsWidget::addTab(
	QWidget *widget,
	QString name,
	bool closable
) {
	int idx = m_tabWidget->addTab(widget, name);
	if (!closable) {
		m_tabWidget->tabBar()->setTabButton(idx, QTabBar::RightSide, 0);
		m_tabWidget->tabBar()->setTabButton(idx, QTabBar::LeftSide, 0);
	}

	m_tabWidget->setCurrentIndex(idx);
}

void TabsWidget::removeTab(int idx) {
	m_tabWidget->removeTab(idx);
}

void TabsWidget::selectWidget(QWidget *widget) {
	m_tabWidget->setCurrentWidget(widget);
}

void TabsWidget::deleteWidget(QWidget* widget) {
	int idx = m_tabWidget->indexOf(widget);
	if (idx != -1) {
		m_tabWidget->removeTab(idx);
	}
}

// Events

void TabsWidget::showEvent(QShowEvent*) {
	emit shown();
}

// Slots

void TabsWidget::onTabClose(int idx) {
	emit tabCloseRequested(idx);
}

