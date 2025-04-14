#ifndef H_TABS_WIDGET
#define H_TABS_WIDGET

#include <vector>
#include <string>

#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QShowEvent>
#include <QVBoxLayout>

#include "widgets/style.h"
#include "widgets/base_widget.h"

class TabsWidget: public BaseWidget {
	Q_OBJECT

public:
	TabsWidget(QWidget*, Style*);
	void addTab(QWidget*, QString, bool);
	void removeTab(int);
	void selectWidget(QWidget*);

signals:
	void shown();
	void tabCloseRequested(int);

protected:
	void showEvent(QShowEvent*) override;

private slots:
	void onTabClose(int);

private:
	QTabWidget *m_tabWidget;
	QVBoxLayout m_layout;
};

#endif
