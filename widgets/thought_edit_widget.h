#ifndef H_THOUGHT_EDIT_WIDGET
#define H_THOUGHT_EDIT_WIDGET

#include <QTextEdit>
#include <QObject>
#include <QString>
#include <QEnterEvent>
#include <QMouseEvent>
#include <string>

#include "widgets/style.h"

class ThoughtEditWidget: public QTextEdit {
	Q_OBJECT

public:
	ThoughtEditWidget(QWidget*, Style*, bool, std::string);

signals:
	void mouseEnter();
	void mouseLeave();
	void focusLost();

protected:
	void enterEvent(QEnterEvent *) override;
	void leaveEvent(QEvent *) override;
	void mousePressEvent(QMouseEvent *) override;
	void keyPressEvent(QKeyEvent *) override;

private:
	void clearFocus();
};

#endif
