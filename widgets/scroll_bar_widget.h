#ifndef H_SCROLL_BAR_WIDGET
#define H_SCROLL_BAR_WIDGET

#include <QWidget>
#include <QPaintEvent>

#include "layout/scroll_area_layout.h"
#include "widgets/base_widget.h"
#include "widgets/style.h"

class ScrollBarWidget: public BaseWidget {
	Q_OBJECT

public:
	ScrollBarWidget(QWidget*, Style*, ScrollBarPos);
	void setSettings(float, float);
	// Event overrides.
	void paintEvent(QPaintEvent*) override;

private:
	ScrollBarPos m_pos;
	float m_barWidth = 0;
	float m_offset = 0;
};	

#endif

