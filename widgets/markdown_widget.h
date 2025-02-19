#ifndef H_MARKDOWN_WIDGET
#define H_MARKDOWN_WIDGET

#include <optional>

#include <QObject>
#include <QWidget>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QResizeEvent>
#include <QSyntaxHighlighter>
#include <QTextDocument>

#include "model/text_model.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"

class MarkdownHighlighter: public QSyntaxHighlighter {
public:
	MarkdownHighlighter(Style*, QTextDocument*);
	// Model.
	void setModel(TextModel*);
	// Highlighting.
	void highlightBlock(const QString&) override;
	// Model updates.
	void onActiveBlockChanged(int);

private:
	TextModel *m_model = nullptr;
	Style *m_style;
	int m_activeBlock = -1;
};

class MarkdownWidget: public QTextEdit {
	Q_OBJECT

public:
	MarkdownWidget(QWidget*, Style*);
	~MarkdownWidget();
	void load(QString);
	void resizeEvent(QResizeEvent*) override;

protected slots:
	void onCursorMoved();

private:
	// State.
	Style *m_style;
	MarkdownHighlighter *m_highlighter;
	TextModel m_model;
	std::optional<QTextCursor> m_prevCursor = std::nullopt;
	// Helpers.
	void formatBlock(QTextBlock, QString*, std::vector<FormatRange>*);
	int adjustForUnfolding(QString*, std::vector<FormatRange>*, int);
};

#endif

