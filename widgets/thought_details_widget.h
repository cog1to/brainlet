#ifndef H_THOUGHT_DETAILS_WIDGET
#define H_THOUGHT_DETAILS_WIDGET

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/markdown_scroll_widget.h"

class ThoughtDetailsWidget: public BaseWidget {
	Q_OBJECT

public:
	ThoughtDetailsWidget(QWidget*, Style*, MarkdownScrollWidget*);
	~ThoughtDetailsWidget();
	void setTitle(QString title);

private:
	QLabel *m_title;
	QWidget *m_separator;
	QVBoxLayout *m_layout;
	MarkdownScrollWidget *m_markdown;
};

#endif
