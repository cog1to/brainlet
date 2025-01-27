#ifndef H_ANCHOR_HIGHLIGHT_WIDGET
#define H_ANCHOR_HIGHLIGHT_WIDGET

#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>

#include "widgets/base_widget.h"
#include "widgets/anchor_widget.h"

class AnchorHighlightWidget: public BaseWidget {
	Q_OBJECT

public:
	AnchorHighlightWidget(QWidget*, Style*);
	void paintEvent(QPaintEvent*) override;
	void resizeEvent(QResizeEvent*) override;

private:
	AnchorWidget m_anchor;
};

#endif
