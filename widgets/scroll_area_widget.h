#ifndef H_SCROLL_AREA_WIDGET
#define H_SCROLL_AREA_WIDGET

#include <QWidget>
#include <QWheelEvent>

#include "layout/scroll_area_layout.h"
#include "widgets/base_widget.h"

class ScrollAreaWidget: public BaseWidget {
	Q_OBJECT

public:
	ScrollAreaWidget(QWidget*, Style*, ScrollBarPos);
	void setScrollBarPos(ScrollBarPos);
	// Events.
	void wheelEvent(QWheelEvent *) override;

private:
	// State.
	ScrollBarPos m_pos;
};

#endif

