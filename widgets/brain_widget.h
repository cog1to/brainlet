#ifndef H_BRAIN_WIDGET
#define H_BRAIN_WIDGET

#include <QWidget>
#include <QHBoxLayout>

#include "widgets/thought_details_widget.h"
#include "widgets/canvas_widget.h"

class BrainWidget: public BaseWidget {
	Q_OBJECT

public:
	BrainWidget(QWidget*, Style*, CanvasWidget*, ThoughtDetailsWidget*);
	ThoughtDetailsWidget *details();

private:
	QHBoxLayout m_layout;
	CanvasWidget *m_canvas;
	ThoughtDetailsWidget *m_details;
};

#endif
