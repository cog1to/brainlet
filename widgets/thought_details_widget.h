#ifndef H_THOUGHT_DETAILS_WIDGET
#define H_THOUGHT_DETAILS_WIDGET

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/markdown_widget.h"

class ThoughtDetailsWidget: public BaseWidget {
	Q_OBJECT

public:
	ThoughtDetailsWidget(QWidget*, Style*, MarkdownWidget*);
	void setTitle(QString title);

private:
	QLabel m_title;
	QWidget m_separator;
	QVBoxLayout m_layout;
	MarkdownWidget *m_markdown;
};

#endif
