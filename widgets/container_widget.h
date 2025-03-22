#ifndef H_CONTAINER_WIDGET
#define H_CONTAINER_WIDGET

#include <vector>

#include <QWidget>
#include <QResizeEvent>

#include "widgets/base_widget.h"
#include "widgets/style.h"
#include "widgets/canvas_widget.h"
#include "widgets/search_widget.h"

class ContainerWidget: public BaseWidget {
	Q_OBJECT

public:
	ContainerWidget(QWidget*, Style*, CanvasWidget*);
	CanvasWidget *canvas();
	SearchWidget *search();
	void resizeEvent(QResizeEvent *) override;

private slots:
	void onSearchCanceled(SearchWidget*);
	void onSearchActivated(SearchWidget*);
	void onSearchUpdated(SearchWidget*);

private:
	CanvasWidget *m_canvas = nullptr;
	SearchWidget m_search;
	// Helpers
	void layoutSearch();
	void updateSearchWidth();
};

#endif
