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
	ThoughtEditWidget(
		QWidget* parent,
		Style* style,
		bool ro,
		std::string text
	);
	void clearFocus();

signals:
	void mouseEnter();
	void mouseLeave();
	void focusLost();
	void editStarted();
	void editCanceled();
	void editConfirmed(std::function<void(bool)>);
	void clicked();
	void menuRequested(const QPoint&);
	void nextSuggestion();
	void prevSuggestion();

protected:
	void enterEvent(QEnterEvent *) override;
	void leaveEvent(QEvent *) override;
	void mousePressEvent(QMouseEvent *) override;
	void keyPressEvent(QKeyEvent *) override;
	bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif
