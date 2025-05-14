#ifndef H_MARKDOWN_EDIT_WIDGET
#define H_MARKDOWN_EDIT_WIDGET

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/markdown_block_widget.h"
#include "model/new_text_model.h"

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

signals:
	void onCursorMove(MarkdownCursor, MarkdownCursor);

protected:
	void resizeEvent(QResizeEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void keyPressEvent(QKeyEvent*) override;

private:
	QVBoxLayout *m_layout;
	QList<MarkdownBlock*> m_blocks;
	MarkdownCursor m_cursor;
	// State.
	text::TextModel m_model;	
	MarkdownBlock *m_activeBlock = nullptr;
	// Helpers.
	MarkdownBlock *blockBefore(MarkdownBlock*);
	MarkdownBlock *blockAfter(MarkdownBlock*);
	bool cursorAtBlockBelow(MarkdownCursor, MarkdownCursor*);
	bool cursorAtBlockAbove(MarkdownCursor, MarkdownCursor*);
	void processCursorMove(MarkdownCursor, MarkdownCursor);
};

#endif

