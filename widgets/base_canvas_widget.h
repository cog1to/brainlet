#ifndef H_BASE_CANVAS_WIDGET
#define H_BASE_CANVAS_WIDGET

#include <unordered_map>
#include <vector>

#include <QWidget>
#include <QResizeEvent>

#include "layout/base_layout.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/thought_widget.h"

class BaseCanvasWidget: public BaseWidget {
	Q_OBJECT

public:
	BaseCanvasWidget(QWidget*, Style*, BaseLayout*);
	void setState(State*);

protected:
	// Event overrides.
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *) override;

private:
	BaseLayout *m_layout = nullptr;
	State *m_state = nullptr;
	std::unordered_map<ThoughtId, ThoughtWidget*> m_widgets;
	// Layout.
	void updateLayout();
	ThoughtWidget *cachedWidget(ThoughtId id);
	ThoughtWidget *createWidget(const Thought*, bool);

private slots:
	void onWidgetActivated(ThoughtWidget*);
	void onWidgetDeactivated(ThoughtWidget*);
};

#endif

