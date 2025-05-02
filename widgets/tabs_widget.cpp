#include <vector>
#include <string>

#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QShowEvent>
#include <QBoxLayout>
#include <QTabBar>
#include <QPushButton>

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
				padding: 8px;\
				padding-left: 16px;\
				padding-right: 16px;\
				font: normal %1px \"%2\";\
				color: %3;\
				background: %4;\
				border-bottom: none;\
				border-top-left-radius: 4px;\
				border-top-right-radius: 4px;\
			}\
			QTabBar::tab:selected {\
				background: %5;\
			}\
			QTabBar::tab:!selected {\
				background: qlineargradient(\
					x1: 0, y1: 0, x2: 1, y2: 0,\
					stop: 0 %6, stop: 0.9 %8, stop: 1.0 %9);\
				color: %7;\
			}\
			QTabBar::tab:!selected:hover {\
				background: %10;\
			}\
			QTabBar::close-button {\
				image: url(:/icons/close.png);\
				subcontrol-position: right;\
				padding: 2px;\
			}\
			QTabBar::close-button:hover {\
				background-color: #99999999;\
				border-radius: 4px;\
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
		.arg(style->background().darker(130).name(QColor::HexRgb))
		.arg(style->background().darker(140).name(QColor::HexRgb))
		.arg(style->background().lighter(120).name(QColor::HexRgb))
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
	} else {
		m_tabWidget->tabBar()->setTabButton(idx, QTabBar::LeftSide, 0);

		//QIcon icon = QIcon(":/icons/close.png");
		//QString empty = "";
		//m_tabWidget->tabBar()->setTabButton(
		//	idx,
		//	QTabBar::RightSide,
		//	new QPushButton(icon, empty)
		//);
	}

	m_tabWidget->setCurrentIndex(idx);
}

void TabsWidget::removeTab(int idx) {
	m_tabWidget->removeTab(idx);
}

void TabsWidget::renameTab(int idx, QString name) {
	m_tabWidget->setTabText(idx, name);
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

