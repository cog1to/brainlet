#ifndef H_SCROLL_BAR_WIDGET
#define H_SCROLL_BAR_WIDGET

#include <QWidget>

#include "widgets/base_widget.h"
#include "widgets/style.h"

class ScrollBarWidget: public BaseWidget {
	Q_OBJECT

public:
	ScrollBarWidget(QWidget*, Style*);
};	

#endif

