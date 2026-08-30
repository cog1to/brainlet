#ifndef H_CONTAINER_WIDGET
#define H_CONTAINER_WIDGET

#include <vector>

#include <QWidget>
#include <QResizeEvent>

#include "widgets/base_widget.h"
#include "widgets/style.h"
#include "widgets/canvas_widget.h"
#include "widgets/search_widget.h"
#include "widgets/history_widget.h"

/**
 * TODO: Rename to something less generic.
 *
 * This widget is a wrapper around main Brain canvas. It holds the CanvasWidget,
 * which draws the nodes, SearchWidget, which is a input field with suggestions
 * at the top of the canvas, and HistoryWidget, which is a list of visited
 * nodes.
 */
class ContainerWidget: public BaseWidget {
	Q_OBJECT

public:
	ContainerWidget(QWidget*, Style*, CanvasWidget*);
	CanvasWidget *canvas();
	SearchWidget *search();
	HistoryWidget *history();
	void resizeEvent(QResizeEvent *) override;

private slots:
	void onSearchCanceled(SearchWidget*);
	void onSearchActivated(SearchWidget*);
	void onSearchUpdated(SearchWidget*);

private:
	CanvasWidget *m_canvas = nullptr;
	SearchWidget m_search;
	HistoryWidget m_history;
	// Helpers
	void layoutSearch();
	void layoutHistory();
	void updateSearchWidth();
};

#endif
