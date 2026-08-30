#ifndef H_BRAIN_WIDGET
#define H_BRAIN_WIDGET

#include <QWidget>
#include <QHBoxLayout>

#include "widgets/thought_details_widget.h"
#include "widgets/container_widget.h"

/**
 * Holds both node graph widget (ContainerWidget with CanvasWidget inside of
 * it), and node "details" widget (ThougtDetailsWidget) that shows content of
 * currently selected node/Thought.
 */
class BrainWidget: public BaseWidget {
	Q_OBJECT

public:
	BrainWidget(QWidget*, Style*, ContainerWidget*, ThoughtDetailsWidget*);
	ThoughtDetailsWidget *details();
	ContainerWidget *canvas();

private:
	QHBoxLayout m_layout;
	ContainerWidget *m_canvasContainer;
	ThoughtDetailsWidget *m_details;
};

#endif
