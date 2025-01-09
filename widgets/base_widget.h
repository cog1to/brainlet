#ifndef H_BASE_WIDGET
#define H_BASE_WIDGET

#include <QObject>
#include <QWidget>

#include "widgets/style.h"

class BaseWidget: public QWidget {
	Q_OBJECT

public:
	BaseWidget(QWidget*, Style*);

protected:
	Style *m_style;
};


#endif
