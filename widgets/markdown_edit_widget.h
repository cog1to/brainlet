#ifndef H_MARKDOWN_EDIT_WIDGET
#define H_MARKDOWN_EDIT_WIDGET

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLine>

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
	// Presenter.
	void setPresenter(MarkdownEditPresenter*);

signals:
	void onCursorMove(MarkdownCursor, MarkdownCursor);
	void cursorMoved(QLine);

protected:
	void resizeEvent(QResizeEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void keyPressEvent(QKeyEvent*) override;

private:
	QVBoxLayout *m_layout = nullptr;
	QList<MarkdownBlock*> m_blocks;
	MarkdownCursor m_cursor;
	MarkdownEditPresenter *m_presenter = nullptr;
	// State.
	text::TextModel m_model;	
	MarkdownBlock *m_activeBlock = nullptr;
	MarkdownSelection m_selection = MarkdownSelection();
	// Selection and clipboard.
	MarkdownCursor deleteSelection();
	void copySelectionToClipboard();
	MarkdownCursor pasteFromClipboard();
	// Helpers.
	MarkdownBlock *blockBefore(MarkdownBlock*);
	MarkdownBlock *blockAfter(MarkdownBlock*);
	bool cursorAtBlockBelow(MarkdownCursor, MarkdownCursor*);
	bool cursorAtBlockAbove(MarkdownCursor, MarkdownCursor*);
	void processCursorMove(MarkdownCursor, MarkdownCursor);
	bool cursorAtPoint(QPoint, MarkdownCursor*);
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
};

class MarkdownEditPresenter {
public:
	virtual int getPageOffset(bool down) = 0;	
};

#endif

