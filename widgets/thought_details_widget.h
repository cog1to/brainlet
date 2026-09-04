#ifndef H_THOUGHT_DETAILS_WIDGET
#define H_THOUGHT_DETAILS_WIDGET

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/markdown_scroll_widget.h"
#include "widgets/wrapping_text_edit_widget.h"

/**
 * Displays Thought title content.
 */
class ThoughtDetailsWidget: public BaseWidget {
	Q_OBJECT

public:
	ThoughtDetailsWidget(QWidget*, Style*, MarkdownScrollWidget*);
	~ThoughtDetailsWidget();
	void setTitle(QString title);

signals:
	void titleEditConfirmed(QString, std::function<void(bool)>);

protected slots:
	void onTitleEditConfirmed(QString&, std::function<void(bool)>);

private:
	WrappingTextEditWidget *m_title;
	QWidget *m_separator;
	QVBoxLayout *m_layout;
	MarkdownScrollWidget *m_markdown;
};

#endif
