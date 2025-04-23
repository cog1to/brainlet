#ifndef H_TOAST_WIDGET
#define H_TOAST_WIDGET

#include <QObject>
#include <QWidget>
#include <QString>
#include <QLabel>
#include <QHBoxLayout>
#include <QFrame>
#include <QMouseEvent>
#include <QTimer>

#include "widgets/style.h"

class ToastWidget: public QFrame {
	Q_OBJECT

public:
	ToastWidget(Style*, QString);
	void show(QWidget*);
	void hide();

protected:
	void mouseReleaseEvent(QMouseEvent*);

private:
	Style *m_style;
	QString m_text;
	QLabel m_label;
	QHBoxLayout m_layout;
};

#endif

