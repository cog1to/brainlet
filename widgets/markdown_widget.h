#ifndef H_MARKDOWN_WIDGET
#define H_MARKDOWN_WIDGET

#include <QObject>
#include <QWidget>
#include <QPlainTextEdit>
#include <QResizeEvent>

#include "widgets/style.h"
#include "widgets/base_widget.h"

class MarkdownWidget: public QPlainTextEdit {
	Q_OBJECT

public:
	MarkdownWidget(QWidget*, Style*);
	void resizeEvent(QResizeEvent*) override;

protected:
	Style *m_style;
};

#endif

