#ifndef H_MARKDOWN_SCROLL_WIDGET
#define H_MARKDOWN_SCROLL_WIDGET

#include <QWidget>
#include <QScrollArea>
#include <QLine>

#include "widgets/style.h"
#include "widgets/markdown_edit_widget.h"
#include "widgets/toast_widget.h"

enum MarkdownScrollError {
	MarkdownScrollIOError
};

class MarkdownScrollWidget
	: public QScrollArea, public MarkdownEditPresenter
{
	Q_OBJECT

public:
	MarkdownScrollWidget(QWidget*, Style*);
	~MarkdownScrollWidget();
	void setMarkdownWidget(MarkdownEditWidget*);
	MarkdownEditWidget *markdownWidget();
	int getPageOffset(bool) override;

public slots:
	void onError(MarkdownScrollError error);

protected slots:
	void onCursorMoved(QLine);

private:
	MarkdownEditWidget *m_widget = nullptr;
	Style *m_style = nullptr;
	// Error.
	ToastWidget *m_error = nullptr;
};

#endif

