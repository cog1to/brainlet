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

#include "widgets/style.h"
#include "model/new_text_model.h"

class MarkdownBlock: public QFrame {
	Q_OBJECT

public:
	MarkdownBlock(QWidget*, Style*);
	// Model.
	void setParagraph(text::Paragraph*);
	// Events.
	void paintEvent(QPaintEvent*) override;
	void resizeEvent(QResizeEvent*) override;
	QSize sizeHint() const override;

private:
	// State.
	QList<QTextLayout*> m_layouts;
	text::Paragraph *m_par = nullptr;
	Style *m_style = nullptr;
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

#endif

