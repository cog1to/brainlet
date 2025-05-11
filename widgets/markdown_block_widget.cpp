#include <QFrame>
#include <QString>
#include <QTextLayout>
#include <QList>
#include <QMargins>
#include <QPainter>

#include "model/new_text_model.h"
#include "widgets/style.h"
#include "widgets/markdown_block_widget.h"

using namespace text;

MarkdownBlock::MarkdownBlock(QWidget *widget, Style *style)
	: QFrame(widget), m_style(style)
{
	setContentsMargins(0, 0, 0, 0);

	// Disable mouse and key events.
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setFocusPolicy(Qt::NoFocus);
}

void MarkdownBlock::setParagraph(Paragraph *par) {
	m_par = par;

	// Clear previous layouts.
	for (auto layout: m_layouts)
		delete layout;
	m_layouts.clear();

	if (m_par == nullptr)
		return;

	QList<Line> *lines = par->getLines();
	for (qsizetype i = 0; i < lines->size(); ++i) {
		QTextLayout *layout = new QTextLayout();
		layout->setCacheEnabled(true);

		if (par->getType() == Code)
			layout->setFont(m_style->codeFont());
		else
			layout->setFont(m_style->textEditFont());

		Line line = lines->at(i);
		layout->setText(line.folded);

		QList<QTextLayout::FormatRange> formats = convertRanges(line.foldedFormats);
		layout->setFormats(formats);

		m_layouts.push_back(layout);
	}

	update();
}

// Events.

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
	qreal height = 0;

	for (qsizetype i = 0; i < m_layouts.size(); ++i) {
		const QTextLayout *item = m_layouts.at(i);
		QTextLayout *layout = (QTextLayout*)item;
		layout->beginLayout();

		while (true) {
			QTextLine line = layout->createLine();
			if (!line.isValid())
				break;

			line.setLineWidth(lineWidth);
			line.setPosition(QPointF(0, height));
			height += line.height();
		}

		layout->endLayout();
	}

	return QSize(
		lineWidth + margins.left() + margins.right()
			+ formatMargins.left() + formatMargins.right(),
		height + margins.top() + margins.bottom()
			+ formatMargins.top() + formatMargins.bottom()
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

	QPainter painter(this);
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
					y + (fontMetrics.ascent() - fontMetrics.xHeight()),
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

		layout->beginLayout();

		while (true) {
			QTextLine line = layout->createLine();
			if (!line.isValid())
				break;

			line.setLineWidth(lineWidth);
			const int nextLineY = y + lineSpacing;

			line.draw(
				&painter,
				QPoint(margins.left() + formatMargins.left(), y)
			);

			y = nextLineY;
		}

		layout->endLayout();
	}
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

