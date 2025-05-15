#ifndef H_MARKDOWN_SCROLL_WIDGET
#define H_MARKDOWN_SCROLL_WIDGET

#include <QWidget>
#include <QScrollArea>
#include <QLine>

#include "widgets/markdown_edit_widget.h"

class MarkdownScrollWidget
	: public QScrollArea, public MarkdownEditPresenter
{
	Q_OBJECT

public:
	MarkdownScrollWidget(QWidget*);
	void setMarkdownWidget(MarkdownEditWidget*);
	int getPageOffset(bool) override;

protected slots:
	void onCursorMoved(QLine);

private:
	MarkdownEditWidget *m_widget = nullptr;
};

#endif

