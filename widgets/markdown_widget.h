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
#include <QKeyEvent>
#include <QMimeData>
#include <QTimer>
#include <QPoint>
#include <QFocusEvent>

#include "model/thought.h"
#include "model/text_model.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"

enum MarkdownError {
	MarkdownIOError
};

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
	// Overrides.
	void insertFromMimeData(const QMimeData*) override;
	QMimeData *createMimeDataFromSelection() const override;
	// Events.
	void resizeEvent(QResizeEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void contextMenuEvent(QContextMenuEvent*) override;
	// State.
	bool isDirty() const;
	QString text() const;
	Style *style();
	// Node search.
	void showSearchWidget(QWidget*, QPoint);
	void hideSearchWidget();
	void insertNodeLink(ThoughtId, QString);

signals:
	void textChanged(QString&);
	void nodeLinkSelected(ThoughtId);
	void nodeInsertionActivated(QPoint);

public slots:
	void onError(MarkdownError);

protected slots:
	void onCursorMoved();
	void saveText();
	void onInsertNodeLink();

private:
	// State.
	Style *m_style;
	MarkdownHighlighter *m_highlighter;
	// Text model and cursor state.
	TextModel m_model;
	int m_prevBlock = -1;
	// Link text under cursor.
	QString m_anchor;
	// Saving timer.
	bool m_isDirty = false;
	QTimer *m_saveTimer = nullptr;
	// Search.
	QWidget *m_search = nullptr;
	QTextCursor m_menuCursor;
	// Helpers.
	void formatBlock(QTextBlock, QString*, std::vector<FormatRange>*);
	int adjustForUnfolding(const QString*, const std::vector<FormatRange>*, int) const;
	QTextCursor append(TextModel, QTextCursor);
	// Keyboard helpers.
	bool isControlKey(QKeyEvent*);
	bool isNewlineKey(QKeyEvent*);
	void deleteSelection(QTextCursor*, std::vector<Line>::iterator*, int*);
	bool isLastListItem(std::vector<Line>::iterator*);
	bool isFirstListItem(std::vector<Line>::iterator*);
	bool isFirstCodeBlock(std::vector<Line>::iterator*);
	bool isLastCodeBlock(std::vector<Line>::iterator*);
	void endListOrCode(QTextCursor*, std::vector<Line>::iterator);
	void deleteListOrCode(QTextCursor*);
	QString getSelection(QTextCursor) const;
	// Saving logic.
	void throttleSave();
	// Metrics.
	static constexpr int ParagraphMargin = 12;
};

#endif

