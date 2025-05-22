#ifndef H_MARKDOWN_EDIT_WIDGET
#define H_MARKDOWN_EDIT_WIDGET

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLine>
#include <QTimer>

#include "model/thought.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/markdown_block_widget.h"
#include "model/new_text_model.h"

class MarkdownEditPresenter;

class MarkdownSelection {
public:
	MarkdownSelection() {};
	// Properties.
	bool active = false;
	MarkdownCursor start = MarkdownCursor(nullptr, nullptr, 0);
	MarkdownCursor end = MarkdownCursor(nullptr, nullptr, 0);
};

class MarkdownEditWidget
	: public BaseWidget, public MarkdownCursorProvider
{
	Q_OBJECT

public:
	MarkdownEditWidget(QWidget*, Style*);
	~MarkdownEditWidget();
	void load(QString);
	// Provider.
	MarkdownCursor *currentCursor() override;
	QTextLayout::FormatRange selectionInLine(
		MarkdownBlock*,
		text::Line*
	) override;
	bool isDocumentEmpty() override;
	// Presenter.
	void setPresenter(MarkdownEditPresenter*);
	// Node search.
	void showSearchWidget(QWidget*, QPoint);
	void hideSearchWidget();
	void insertNodeLink(ThoughtId, QString);
	// State.
	bool isDirty() const;
	QString text();
	// Style.
	Style *style();

signals:
	void onCursorMove(MarkdownCursor, MarkdownCursor);
	void cursorMoved(QLine, bool);
	void nodeInsertionActivated(QPoint);
	void textChanged(QString&);
	void nodeLinkSelected(ThoughtId);

protected:
	void resizeEvent(QResizeEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void keyPressEvent(QKeyEvent*) override;

protected slots:
	void saveText();
	void onInsertNodeLink();

private:
	QVBoxLayout *m_layout = nullptr;
	QList<MarkdownBlock*> m_blocks = {};
	MarkdownCursor m_cursor = MarkdownCursor(nullptr, nullptr, 0);
	MarkdownEditPresenter *m_presenter = nullptr;
	// State.
	text::TextModel m_model = text::TextModel();
	MarkdownBlock *m_activeBlock = nullptr;
	MarkdownSelection m_selection = MarkdownSelection();
	QString m_anchor = "";
	QPoint m_pressPoint = QPoint(0, 0);
	// Saving text.
	bool m_isDirty = false;
	QTimer *m_saveTimer = nullptr;
	// Search.
	QWidget *m_search = nullptr;
	// Selection and clipboard.
	MarkdownCursor deleteSelection();
	void copySelectionToClipboard();
	MarkdownCursor pasteFromClipboard();
	MarkdownCursor pasteString(QString);
	// Saving.
	void throttleSave();
	// Links handling.
	void checkForLinksUnderCursor(MarkdownCursor);
	void onAnchorClicked(QString);
	// Helpers.
	MarkdownBlock *blockBefore(MarkdownBlock*);
	MarkdownBlock *blockAfter(MarkdownBlock*);
	bool cursorAtBlockBelow(MarkdownCursor, MarkdownCursor*);
	bool cursorAtBlockAbove(MarkdownCursor, MarkdownCursor*);
	void processCursorMove(MarkdownCursor, MarkdownCursor);
	bool cursorAtPoint(QPoint, MarkdownCursor*);
	bool cursorAbovePoint(QPoint, MarkdownCursor*);
	MarkdownCursor moveCursor(int, MarkdownCursor);
	MarkdownCursor documentStart();
	MarkdownCursor documentEnd();
	inline int indexOfParagraph(text::Paragraph*);
	inline text::Paragraph *insertParagraph(int index, text::Paragraph);
	inline void deleteParagraph(int index);
	void mergeBlocks(int next, text::Line *line, MarkdownCursor prev);
	MarkdownCursor splitBlocks(MarkdownCursor cursor, bool shiftUsed);
	MarkdownCursor cursorAtPoint(QPoint, bool*);
	bool isMovementKey(QKeyEvent*);
	MarkdownCursor adjustForUnfolding(MarkdownCursor, MarkdownCursor);
	bool cursorAfter(MarkdownCursor, MarkdownCursor);
	void showContextMenu(QMouseEvent*);
};

class MarkdownEditPresenter {
public:
	virtual int getPageOffset(bool down) = 0;
};

#endif

