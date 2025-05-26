#ifndef H_TABS_WIDGET
#define H_TABS_WIDGET

#include <vector>

#include <QObject>
#include <QString>
#include <QWidget>
#include <QTabWidget>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QKeyEvent>

#include "widgets/style.h"
#include "widgets/base_widget.h"

class TabsWidget: public BaseWidget {
	Q_OBJECT

public:
	TabsWidget(QWidget*, Style*);
	void addTab(QWidget*, QString, bool);
	void removeTab(int);
	void renameTab(int, QString);
	void selectWidget(QWidget*);
	void deleteWidget(QWidget*);

signals:
	void shown();
	void tabCloseRequested(int);
	void closeRequested();

protected:
	void showEvent(QShowEvent*) override;
	void closeEvent(QCloseEvent*) override;

private slots:
	void onTabClose(int);

private:
	QTabWidget *m_tabWidget;
	QVBoxLayout m_layout;
};

#endif
