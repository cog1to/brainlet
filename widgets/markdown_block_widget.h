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
	~MarkdownBlock();
	// Model.
	void setParagraph(text::Paragraph*);
	void updateParagraphWithoutReload(text::Paragraph*);
	text::Paragraph *paragraph();
	void setPlaceholder(QString);
	// Cursor positioning.
	bool cursorAt(QPoint, MarkdownCursor*);
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
	QString m_placeholder = "";
	// Helpers.
	QList<QTextLayout::FormatRange> convertRanges(
		QList<text::FormatRange>
	);
	QTextCharFormat qtFormat(
		text::FormatRange,
		Style*,
		QTextCharFormat
	);
	inline int indexOfLine(text::Line*);
	// Layout constants.
	static constexpr QMargins codeMargins = QMargins(10, 10, 10, 10);
	static constexpr QMargins listMargins = QMargins(40, 0, 0, 0);
};

class MarkdownCursor {
public:
	MarkdownCursor(MarkdownBlock*, int, int);
	inline bool operator==(const MarkdownCursor rhs) const {
		return block == rhs.block && line == rhs.line && position == rhs.position;
	}
	inline bool operator!=(const MarkdownCursor rhs) const {
		return block != rhs.block || line != rhs.line || position != rhs.position;
	}
	// Properties
	MarkdownBlock *block;
	int line;
	int position;
};

class MarkdownCursorProvider {
public:
	virtual MarkdownCursor *currentCursor() = 0;
	virtual QTextLayout::FormatRange selectionInLine(
		MarkdownBlock*,
		text::Line*
	) = 0;
	virtual bool isDocumentEmpty() = 0;
};

#endif

