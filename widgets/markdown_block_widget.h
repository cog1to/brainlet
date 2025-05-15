#ifndef H_MARKDOWN_BLOCK
#define H_MARKDOWN_BLOCK

#include <QFrame>
#include <QString>
#include <QTextLayout>
#include <QList>
#include <QTextCharFormat>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMargins>
#include <QLine>

#include "widgets/style.h"
#include "model/new_text_model.h"

// Forward declaration to avoid circula reference compilation errors.
class MarkdownCursorProvider;
class MarkdownCursor;

class MarkdownBlock: public QFrame {
	Q_OBJECT

public:
	MarkdownBlock(QWidget*, Style*, MarkdownCursorProvider*);
	// Model.
	void setParagraph(text::Paragraph*);
	text::Paragraph *paragraph();
	// Cursor positioning.
	bool cursorAt(QPoint, MarkdownCursor*);
	text::Line *lineBefore(text::Line*);
	text::Line *lineAfter(text::Line*);
	bool cursorBelow(MarkdownCursor, MarkdownCursor*);
	bool cursorAbove(MarkdownCursor, MarkdownCursor*);
	qreal xAtCursor(MarkdownCursor);
	MarkdownCursor firstCursorAtX(qreal);
	MarkdownCursor lastCursorAtX(qreal);
	QLine lineForCursor(MarkdownCursor);
	QPoint pointAtCursor(MarkdownCursor);
	// Events.
	void paintEvent(QPaintEvent*) override;
	void resizeEvent(QResizeEvent*) override;
	// Geometry.
	QSize sizeHint() const override;

public slots:
	void onCursorMove(MarkdownCursor, MarkdownCursor);

private:
	// State.
	QList<QTextLayout*> m_layouts;
	text::Paragraph *m_par = nullptr;
	Style *m_style = nullptr;
	MarkdownCursorProvider *m_provider = nullptr;
	// Helpers.
	QList<QTextLayout::FormatRange> convertRanges(
		QList<text::FormatRange> 
	);
	QTextCharFormat qtFormat(
		text::FormatRange,
		Style*,
		QTextCharFormat
	);
	// Layout constants.
	static constexpr QMargins codeMargins = QMargins(10, 10, 10, 10);
	static constexpr QMargins listMargins = QMargins(40, 0, 0, 0);
};

class MarkdownCursor {
public:
	MarkdownCursor(
		MarkdownBlock*,
		text::Line*,
		int
	);
	// Properties
	MarkdownBlock *block;
	text::Line *line;
	int position;
};

class MarkdownCursorProvider {
public:
	virtual MarkdownCursor *currentCursor() = 0;
};

#endif

