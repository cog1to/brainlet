#include <QPlainTextEdit>
#include <QTextBlock>
#include <QString>
#include <QStringList>
#include <QTimer>

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
		.arg(style->textFont().bold() ? "bold" : "")
		.arg(style->textFont().pixelSize())
		.arg(style->textFont().family())
	);

	// Install highlighter.
	m_highlighter = new MarkdownHighlighter(style, document());

	// Cursor movement.
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
}

MarkdownWidget::~MarkdownWidget() {
	delete m_highlighter;
}

void MarkdownWidget::load(QString data) {
	QStringList lines = data.split("\n");
	m_model = TextModel(lines);
	m_highlighter->setModel(&m_model);
	setPlainText(m_model.folded());
}

void MarkdownWidget::resizeEvent(QResizeEvent* event) {
	QPlainTextEdit::resizeEvent(event);
}

// Cursor.

void MarkdownWidget::onCursorMoved() {
	QTextCursor cursor = textCursor();
	int position = cursor.position();
	int posInBlock = cursor.positionInBlock();
	if (m_prevCursor.has_value() && m_prevCursor.value().block() == cursor.block())
		return;

	QTextBlock block = cursor.block();
	int number = block.blockNumber();
	m_highlighter->onActiveBlockChanged(number);

	// Suspend cursor changes while we're editing.
	disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

	// Change previous block.
	if (m_prevCursor.has_value()) {
		QTextBlock prevBlock = m_prevCursor.value().block();
		Line prev = m_model.lines()[prevBlock.blockNumber()];
		formatBlock(prevBlock, &prev.folded, &prev.foldedFormats);
		// Adjust original cursor position to accout for folding.
		if (number > prevBlock.blockNumber())
			position -= prev.text.size() - prev.folded.size();
	}

	// Save prev cursor.
	m_prevCursor = cursor;

	// Update current block.
	if (number >= 0) {
		Line line = m_model.lines()[number];
		formatBlock(cursor.block(), &line.text, &line.formats);
		position += adjustForUnfolding(&line.folded, &line.foldedFormats, posInBlock);
	}

	// Restore cursor.
	QTextCursor newCursor = QTextCursor(this->document());
	newCursor.setPosition(position);
	if (newCursor.position() >= 0) {
		this->setTextCursor(newCursor);
	}

	// Resume cursor updates.
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
}

int MarkdownWidget::adjustForUnfolding(
	QString *text,
	std::vector<FormatRange> *ranges,
	int positionInBlock
) {
	assert(text != nullptr);
	assert(ranges != nullptr);

	int offset = 0;
	for (auto& range: *ranges) {
		if (range.to < positionInBlock) {
			offset += range.endOffset();
		}
		if (range.from < positionInBlock) {
			offset += range.startOffset();
		}
	}

	return offset;
}

void MarkdownWidget::formatBlock(
	QTextBlock block,
	QString *text,
	std::vector<FormatRange> *formats
) {
	assert(text != nullptr);
	assert(formats != nullptr);

	QTextCursor edit(block);
	int size = text->size();

	edit.beginEditBlock();
	edit.select(QTextCursor::LineUnderCursor);
	edit.insertText(*text, QTextCharFormat());
	edit.endEditBlock();
}

// Highlighter logic.

MarkdownHighlighter::MarkdownHighlighter(Style *style, QTextDocument *doc)
	: QSyntaxHighlighter(doc), m_style(style) {}

void MarkdownHighlighter::setModel(TextModel *model) {
	m_model = model;
}

void MarkdownHighlighter::onActiveBlockChanged(int number) {
	m_activeBlock = number;
}

void MarkdownHighlighter::highlightBlock(const QString &text) {
	if (m_model == nullptr)
		return;

	QTextBlock block = currentBlock();
	int number = block.blockNumber();
	if (number == -1)
		return;

	Line line = m_model->lines()[number];

	std::vector<FormatRange> ranges = (m_activeBlock == number) ? line.formats : line.foldedFormats;

	for (auto fmt: ranges) {
		setFormat(
			fmt.from,
			fmt.to - fmt.from,
			fmt.qtFormat(m_style, format(fmt.from))
		);
	}
}

