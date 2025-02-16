#include <QPlainTextEdit>
#include <QTextBlock>

#include "widgets/markdown_widget.h"

#include <QDebug>

MarkdownWidget::MarkdownWidget(QWidget *parent, Style *style)
	: QPlainTextEdit(parent)
{
	m_style = style;
	setWordWrapMode(QTextOption::WordWrap);
	setLineWrapMode(QPlainTextEdit::WidgetWidth);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// Apply style.
	setStyleSheet(
		QString("background-color: %1; font: %2 %3px \"%4\"")
		.arg(style->background().name(QColor::HexRgb))
		.arg(style->font().bold() ? "bold" : "")
		.arg(style->font().pixelSize())
		.arg(style->font().family())
	);
}

void MarkdownWidget::resizeEvent(QResizeEvent* event) {
	QPlainTextEdit::resizeEvent(event);

	// enumerate text blocks
	QTextBlock block = document()->begin();
	int idx = 0;
	while (block != document()->end()) {
		qDebug() << idx << ": '" << block.text() << "'\n";
		block = block.next();
		idx += 1;
	}
}

