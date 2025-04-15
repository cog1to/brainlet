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
	setStyleSheet(
		QString("TabsWidget { background-color: %1; }")
		.arg(style->background().darker(150).name(QColor::HexRgb))
	);

	m_tabWidget = new QTabWidget(nullptr);
	m_tabWidget->setTabsClosable(true);
	m_tabWidget->setElideMode(Qt::ElideRight);
	m_tabWidget->setStyleSheet(
		QString("QTabBar::tab {\
				padding: 4px;\
				padding-left: 12px;\
				padding-right: 12px;\
				font: normal %1px \"%2\";\
				color: %3;\
				background: %4;\
				border-top-left-radius: 4px;\
				border-top-right-radius: 4px;\
			}\
			QTabBar::tab {\
				border-bottom: none;\
			}\
			QTabBar::tab:selected, QTabBar::tab:hover {\
				background: %5;\
			}\
			QTabBar::tab:!selected {\
				background: %6;\
				border-bottom: none;\
				color: %7;\
			}\
			QTabWidget::tab-bar {\
				alignment: left;\
				background: black;\
			}\
			QTabWidget::pane {\
				border-bottom-left-radius: 8px;\
				border-bottom-right-radius: 8px;\
				background-color: %4;\
				padding-top: 4px;\
			}"
		)
		.arg(style->font().pixelSize())
		.arg(style->font().family())
		.arg(style->textEditColor().name(QColor::HexRgb))
		.arg(style->background().name(QColor::HexRgb))
		.arg(style->background().name(QColor::HexRgb))
		.arg(style->background().darker(130).name(QColor::HexRgb))
		.arg(style->textColor().darker(120).name(QColor::HexRgb))
	);
	// TODO: Customize/subclass QTabBar. Looks out of place on Mac.

	m_layout.setContentsMargins(QMargins(8,16,8,8));
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

