#ifndef H_SCROLL_AREA_WIDGET
#define H_SCROLL_AREA_WIDGET

#include <QWidget>
#include <QWheelEvent>
#include <QResizeEvent>

#include "layout/scroll_area_layout.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/scroll_bar_widget.h"

class ScrollAreaWidget: public BaseWidget {
	Q_OBJECT

public:
	ScrollAreaWidget(QWidget*, Style*, unsigned int, ScrollBarPos);
	void setScrollBarPos(ScrollBarPos);
	void setScrollBarSettings(float, float);
	// Events.
	void wheelEvent(QWheelEvent *) override;
	void resizeEvent(QResizeEvent *) override;

signals:
	void scrolled(unsigned int, int);

private:
	// Subviews.
	ScrollBarWidget m_bar;
	// State.
	ScrollBarPos m_pos;
	unsigned int m_id;
};

#endif

