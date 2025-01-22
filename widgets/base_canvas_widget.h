#ifndef H_BASE_CANVAS_WIDGET
#define H_BASE_CANVAS_WIDGET

#include <unordered_map>
#include <vector>

#include <QWidget>
#include <QResizeEvent>
#include <QWheelEvent>

#include "layout/base_layout.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/thought_widget.h"
#include "widgets/scroll_area_widget.h"

class BaseCanvasWidget: public BaseWidget {
	Q_OBJECT

public:
	BaseCanvasWidget(QWidget*, Style*, BaseLayout*);
	~BaseCanvasWidget();
	void setState(State*);

protected:
	// Event overrides.
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *) override;

private:
	BaseLayout *m_layout = nullptr;
	State *m_state = nullptr;
	std::unordered_map<ThoughtId, ThoughtWidget*> m_widgets;
	std::unordered_map<unsigned int, ScrollAreaWidget*> m_scrollAreas;
	// Layout.
	void updateLayout();
	void layoutScrollAreas();
	ThoughtWidget *cachedWidget(ThoughtId id);
	ThoughtWidget *createWidget(const Thought*, bool);
	ScrollAreaWidget *cachedScrollArea(unsigned int id);
	ScrollAreaWidget *createScrollArea(unsigned int id, ScrollBarPos);

private slots:
	void onWidgetActivated(ThoughtWidget*);
	void onWidgetDeactivated(ThoughtWidget*);
	void onWidgetScroll(ThoughtWidget*, QWheelEvent*);
	void onScrollAreaScroll(unsigned int, int);
};

#endif

