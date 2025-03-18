#ifndef H_LINK_BUTTON_WIDGET
#define H_LINK_BUTTON_WIDGET

#include <QEvent>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QPushButton>

#include "widgets/style.h"

class LinkButtonWidget: public QPushButton {
	Q_OBJECT

public:
	LinkButtonWidget(QWidget*, Style*);
	// Event overrides.
	void enterEvent(QEnterEvent *) override;
	void leaveEvent(QEvent *) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

private:
	Style *m_style;
};

#endif
