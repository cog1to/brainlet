#ifndef H_THOUGHT_EDIT_WIDGET
#define H_THOUGHT_EDIT_WIDGET

#include <QTextEdit>
#include <QObject>
#include <QString>
#include <QEnterEvent>
#include <string>

#include "widgets/style.h"

class ThoughtEditWidget: public QTextEdit {
	Q_OBJECT

public:
	ThoughtEditWidget(QWidget*, Style*, std::string);

signals:
	void mouseEnter();
	void mouseLeave();

protected:
	void enterEvent(QEnterEvent *) override;
	void leaveEvent(QEvent *) override;
};

#endif
