#include <QFrame>
#include <QString>
#include <QTextLayout>
#include <QList>
#include <QMargins>
#include <QPainter>
#include <QLine>

#include "model/new_text_model.h"
#include "widgets/style.h"
#include "widgets/markdown_block_widget.h"

using namespace text;

MarkdownCursor::MarkdownCursor(
	MarkdownBlock *_block,
	text::Line *_line,
	int _pos
) : block(_block), line(_line), position(_pos) {}

MarkdownBlock::MarkdownBlock(
	QWidget *widget,
	Style *style,
	MarkdownCursorProvider *provider
)
	: QFrame(widget), m_style(style), m_provider(provider)
{
	setContentsMargins(0, 0, 0, 0);

	// Disable mouse and key events.
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setFocusPolicy(Qt::NoFocus);
}

text::Paragraph *MarkdownBlock::paragraph() {
	return m_par;
}

void MarkdownBlock::setParagraph(Paragraph *par) {
	m_par = par;

	// Clear previous layouts.
	for (auto layout: m_layouts)
		delete layout;
	m_layouts.clear();

	if (m_par == nullptr)
		return;

	MarkdownCursor *cursor = m_provider->currentCursor();
	text::ParagraphType type = par->getType();

	QColor color = type == text::Code 
		? m_style->codeBackground()
		: m_style->background();
	setStyleSheet(
		QString("background-color: %1").arg(color.name(QColor::HexRgb))
	);

	QList<Line> *lines = par->getLines();
	for (qsizetype i = 0; i < lines->size(); ++i) {
		QTextLayout *layout = new QTextLayout();
		layout->setCacheEnabled(true);

		if (par->getType() == Code)
			layout->setFont(m_style->codeFont());
		else
			layout->setFont(m_style->textEditFont());

		Line line = lines->at(i);
		if (cursor != nullptr && cursor->block == this && cursor->line == &((*lines)[i])) {
			QList<QTextLayout::FormatRange> formats = convertRanges(line.formats);
			layout->setText(line.text);
			layout->setFormats(formats);
		} else {
			QList<QTextLayout::FormatRange> formats = convertRanges(line.foldedFormats);
			layout->setText(line.folded);
			layout->setFormats(formats);
		}

		m_layouts.push_back(layout);
	}

	update();
}

// Cursor.

bool MarkdownBlock::cursorAt(QPoint point, MarkdownCursor *out) {
	QMargins formatMargins = QMargins(0, 0, 0, 0);
	text::ParagraphType type = m_par->getType();
	if (type == text::Code) {
		formatMargins = codeMargins;
	} else if (type == text::BulletList || type == text::NumberList) {
		formatMargins = listMargins;
	}

	for (int idx = 0; idx < m_layouts.size(); idx++) {
		QTextLayout *layout = m_layouts[idx];
		text::Line *parLine = &((*m_par->getLines())[idx]);

		QPointF layoutPosition = layout->position();
		QPointF pos = QPointF(
			point.x() - layoutPosition.x(),
			point.y() - layoutPosition.y()
		);

		if (!layout->boundingRect().contains(pos))
			continue;

		for (int lineIdx = 0; lineIdx < layout->lineCount(); lineIdx++) {
			QTextLine line = layout->lineAt(lineIdx);
			QRectF rect = QRectF(
				line.position().x(),
				line.position().y(),
				line.rect().width(),
				line.rect().height()
			);

			if (rect.contains(pos)) {
				int idx = line.xToCursor(pos.x());
				out->block = this;
				out->line = parLine;
				out->position = idx;
				return true;
			}
		}
	}

	return false;
}

text::Line *MarkdownBlock::lineBefore(Line* line) {
	if (line == nullptr)
		return nullptr;

	QList<text::Line> *lines = m_par->getLines();
	int idx = lines->indexOf(*line);

	if (idx <= 0)
		return nullptr;

	return &((*lines)[idx - 1]);
}

text::Line *MarkdownBlock::lineAfter(Line* line) {
	if (line == nullptr)
		return nullptr;

	QList<text::Line> *lines = m_par->getLines();
	int idx = lines->indexOf(*line);

	if (idx < 0 || idx == lines->size() - 1)
		return nullptr;

	return &((*lines)[idx + 1]);
}

bool MarkdownBlock::cursorBelow(
	MarkdownCursor cur,
	MarkdownCursor *result
) {
	if (cur.block != this)
		return false;

	QList<text::Line> *lines = m_par->getLines();
	int idx = lines->indexOf(*cur.line);
	if (idx == -1)
		return false;

	QTextLayout *layout = m_layouts[idx];
	QTextLine textLine = layout->lineForTextPosition(cur.position);
	qreal x = textLine.cursorToX(cur.position);
	if (textLine.lineNumber() < layout->lineCount() - 1) {
		// We have next line in current layout to move to.
		// Calculate the new cursor position for the same X offset.
		QTextLine nextLine = layout->lineAt(textLine.lineNumber() + 1);
		int pos = nextLine.xToCursor(x);
		result->position = pos;
		return true;
	} else if (idx < m_layouts.size() - 1) {
		// We have next layout to move to.
		// Calculate the new cursor position in that layout for the same X.
		QTextLayout *nextLayout = m_layouts[idx + 1];
		QTextLine nextLine = nextLayout->lineAt(0);
		int pos = nextLine.xToCursor(x);
		result->line = &((*lines)[idx + 1]);
		result->position = pos;
		return true;
	}

	return false;
}

bool MarkdownBlock::cursorAbove(
	MarkdownCursor cur,
	MarkdownCursor *result
) {
	if (cur.block != this)
		return false;

	QList<text::Line> *lines = m_par->getLines();
	int idx = lines->indexOf(*cur.line);
	if (idx == -1)
		return false;

	QTextLayout *layout = m_layouts[idx];
	QTextLine textLine = layout->lineForTextPosition(cur.position);
	qreal x = textLine.cursorToX(cur.position);
	if (textLine.lineNumber() > 0) {
		// We have prev line in current layout to move to.
		// Calculate the new cursor position for the same X offset.
		QTextLine nextLine = layout->lineAt(textLine.lineNumber() - 1);
		int pos = nextLine.xToCursor(x);
		result->position = pos;
		return true;
	} else if (idx > 0) {
		// We have prev layout to move to.
		// Calculate the new cursor position in that layout for the same X.
		QTextLayout *nextLayout = m_layouts[idx - 1];
		QTextLine nextLine = nextLayout->lineAt(nextLayout->lineCount() - 1);
		int pos = nextLine.xToCursor(x);
		result->line = &((*lines)[idx - 1]);
		result->position = pos;
		return true;
	}

	return false;
}

qreal MarkdownBlock::xAtCursor(MarkdownCursor cur) {
	if (cur.block != this)
		return false;

	QList<text::Line> *lines = m_par->getLines();
	int idx = lines->indexOf(*cur.line);
	if (idx == -1)
		return false;

	QTextLayout *layout = m_layouts[idx];
	QTextLine textLine = layout->lineForTextPosition(cur.position);
	qreal x = textLine.cursorToX(cur.position);

	return x;
}

MarkdownCursor MarkdownBlock::firstCursorAtX(qreal x) {
	MarkdownCursor cursor(nullptr, nullptr, 0);

	if (m_layouts.size() == 0)
		return cursor;

	QTextLayout *layout = m_layouts[0];
	QTextLine line = layout->lineAt(0);
	int pos = line.xToCursor(x);

	cursor.block = this;
	cursor.line = &((*m_par->getLines())[0]);
	cursor.position = pos;

	return cursor;
}

MarkdownCursor MarkdownBlock::lastCursorAtX(qreal x) {
	MarkdownCursor cursor(nullptr, nullptr, 0);

	if (m_layouts.size() == 0)
		return cursor;

	int lastLineIdx = m_layouts.size() - 1;
	QTextLayout *layout = m_layouts[lastLineIdx];
	QTextLine line = layout->lineAt(layout->lineCount() - 1);
	int pos = line.xToCursor(x);

	cursor.block = this;
	cursor.line = &((*m_par->getLines())[lastLineIdx]);
	cursor.position = pos;

	return cursor;
}

// Drawing and size.

QSize MarkdownBlock::sizeHint() const {
	QMargins margins = contentsMargins();
	QMargins formatMargins = QMargins(0, 0, 0, 0);
	text::ParagraphType type = m_par->getType();

	QFontMetrics fontMetrics = QFontMetrics(m_style->textEditFont());
	if (type == text::Code) {
		fontMetrics = QFontMetrics(m_style->codeFont());
		formatMargins = codeMargins;
	} else if (type == text::BulletList || type == text::NumberList) {
		formatMargins = listMargins;
	}

	int lineWidth = size().width()
		- margins.left() - margins.right()
		- formatMargins.left() - formatMargins.right();
	qreal height = margins.top() + formatMargins.top();
	qreal start = margins.left() + formatMargins.left();

	for (qsizetype i = 0; i < m_layouts.size(); i++) {
		qreal lineY = 0;
		const QTextLayout *item = m_layouts.at(i);
		QTextLayout *layout = (QTextLayout*)item;
		layout->setPosition(QPoint(start, height));
		layout->beginLayout();

		while (true) {
			QTextLine line = layout->createLine();
			if (!line.isValid())
				break;

			line.setLineWidth(lineWidth);
			line.setPosition(QPointF(0, lineY));
			height += line.height();
			lineY += line.height();
		}

		layout->endLayout();
	}

	return QSize(
		lineWidth + margins.left() + margins.right()
			+ formatMargins.left() + formatMargins.right(),
		height + margins.bottom() + formatMargins.bottom()
	);
}

void MarkdownBlock::resizeEvent(QResizeEvent *event) {
	QFrame::resizeEvent(event);
	updateGeometry();
}

void MarkdownBlock::paintEvent(QPaintEvent *event) {
	QMargins margins = contentsMargins();
	QMargins formatMargins = QMargins(0, 0, 0, 0);
	text::ParagraphType type = m_par->getType();
	MarkdownCursor *cursor = m_provider->currentCursor();
	QList<text::Line> *lines = m_par->getLines();

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(m_style->textEditColor());
	painter.setBrush(m_style->textEditColor());

	QFontMetrics fontMetrics = QFontMetrics(m_style->textEditFont());
	if (type == text::Code) {
		painter.setFont(m_style->codeFont());
		fontMetrics = QFontMetrics(m_style->codeFont());
		formatMargins = codeMargins;
	} else if (type == text::BulletList || type == text::NumberList) {
		painter.setFont(m_style->textEditFont());
		formatMargins = listMargins;
	}

	int lineSpacing = fontMetrics.lineSpacing();
	int lineWidth = size().width()
		- margins.left() - margins.right()
		- formatMargins.left() - formatMargins.right();
	qreal y = margins.top() + formatMargins.top();

	for (qsizetype i = 0; i < m_layouts.size(); ++i) {
		const QTextLayout *item = m_layouts.at(i);
		QTextLayout *layout = (QTextLayout*)item;

		if (type == text::BulletList) {
			painter.drawEllipse(
				QRectF(
					(formatMargins.left() - fontMetrics.xHeight()) / 2,
					y + (fontMetrics.ascent() - fontMetrics.xHeight()) - 1,
					fontMetrics.xHeight(),
					fontMetrics.xHeight()
				)
			);
		} else if (type == text::NumberList) {
			QString number = QString("%1.").arg(i + 1);
			painter.drawText(
				QPointF(
					(formatMargins.left() - fontMetrics.xHeight()) / 2,
					y + fontMetrics.lineSpacing() - fontMetrics.descent()
				), 
				number
			);
		}

		layout->draw(&painter, QPointF(0, 0));
	
		if (
			cursor != nullptr &&
			cursor->block == this &&
			cursor->line == &((*lines)[i])
		) {
			layout->drawCursor(&painter, QPointF(0, 0), cursor->position, 1);
		}

		y += layout->boundingRect().height();
	}
}

// State.

void MarkdownBlock::onCursorMove(
	MarkdownCursor from,
	MarkdownCursor to
) {
	// Ignore when cursor not leaves neither enters this block.
	if (
		(from.block == nullptr || from.block != this) && 
		(to.block == nullptr || to.block != this)
	) {
		return;
	}

	QList<text::Line> *lines = m_par->getLines();

	if (from.line != nullptr) {
		for (auto it = lines->begin(); it != lines->end(); it++) {
			if (from.line == &(*it)) {
				setParagraph(m_par); // TODO: don't relayout everything?
				return;
			}
		}
	}

	if (to.line != nullptr) {
		for (auto it = lines->begin(); it != lines->end(); it++) {
			if (to.line == &(*it)) {
				setParagraph(m_par); // TODO: don't relayout everything?
				return;
			}
		}
	}
}

QLine MarkdownBlock::lineForCursor(MarkdownCursor cursor) {
	int idx = 0;

	if (cursor.block != this)
		return QLine(0, 0, 0, 0);

	QList<text::Line> *lines = m_par->getLines();
	for (idx = 0; idx < lines->size(); idx++) {
		if (&(*lines)[idx] == cursor.line)
			break;
	}

	QTextLayout *layout = m_layouts[idx];
	QTextLine line = layout->lineForTextPosition(cursor.position);
	qreal x = line.cursorToX(cursor.position);

	QPointF pos = layout->position();
	QRect geo = geometry();
	qreal height = line.height();
	return QLine(
		QPoint(geo.x() + x, geo.y() + pos.y() + line.y() - height * 0.5),
		QPoint(geo.x() + x, geo.y() + pos.y() + line.y() + height * 1.5)
	);
}

// Helpers.

QList<QTextLayout::FormatRange> MarkdownBlock::convertRanges(
	QList<FormatRange> from
) {
	QList<QTextLayout::FormatRange> result;

	QTextCharFormat defaultFormat;
	defaultFormat.setFont(m_style->textEditFont());

	for (auto fmt: from) {
		result.push_back(
			QTextLayout::FormatRange{
				.start = fmt.from,
				.length = fmt.to - fmt.from,
				.format = qtFormat(fmt, m_style, defaultFormat)
			}
		);
	}

	return result;
}

QTextCharFormat MarkdownBlock::qtFormat(
	text::FormatRange range,
	Style *style,
	QTextCharFormat fmt
) {
	text::BlockFormat format = range.format;

	switch (format) {
		case text::Heading1:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 2.0);
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading2:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.8);
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading3:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.6);
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading4:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.4);
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading5:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.2);
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading6:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.1);
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Italic:
			fmt.setFontItalic(true);
			break;
		case text::Bold:
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::BoldItalic:
			fmt.setFontItalic(true);
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::CodeSpan:
			fmt.setBackground(style->codeBackground());
			fmt.setFont(style->codeFont());
			break;
		case text::NodeLink:
			fmt.setFontUnderline(true);
			fmt.setForeground(style->borderColor());
			fmt.setAnchor(true);
			fmt.setAnchorHref(range.link.target);
			break;
		case text::Link:
		case text::PlainLink:
			fmt.setFontUnderline(true);
			fmt.setForeground(style->linkColor());
			fmt.setAnchor(true);
			fmt.setAnchorHref(range.link.target);
			break;
		case text::Escape:
			break;
	}

	return fmt;
}

