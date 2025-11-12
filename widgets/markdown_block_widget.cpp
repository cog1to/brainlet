#include <QFrame>
#include <QString>
#include <QTextLayout>
#include <QList>
#include <QMargins>
#include <QPainter>
#include <QLine>
#include <QStack>

#include "model/new_text_model.h"
#include "widgets/style.h"
#include "widgets/markdown_block_widget.h"

using namespace text;

MarkdownCursor::MarkdownCursor(
	MarkdownBlock *_block,
	int _line,
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

MarkdownBlock::~MarkdownBlock() {
	for (auto *layout: m_layouts)
		delete layout;
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

	MarkdownCursor *cursor = nullptr;
	if (m_provider != nullptr)
		cursor = m_provider->currentCursor();

	text::ParagraphType type = par->getType();

	QColor color = type == text::Code
		? m_style->codeBackground()
		: m_style->background();
	setStyleSheet(
		QString("background-color: %1").arg(color.name(QColor::HexRgb))
	);

	QTextOption opt = QTextOption();
	opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

	QList<Line> *lines = par->getLines();
	assert(lines->size() > 0);

	for (qsizetype i = 0; i < lines->size(); ++i) {
		QTextLayout *layout = new QTextLayout();
		layout->setTextOption(opt);
		layout->setCacheEnabled(true);

		if (par->getType() == Code)
			layout->setFont(m_style->codeFont());
		else
			layout->setFont(m_style->textEditFont());

		Line line = lines->at(i);
		if (cursor != nullptr && cursor->block == this && cursor->line == i) {
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

void MarkdownBlock::updateParagraphWithoutReload(
	text::Paragraph* par
) {
	m_par = par;
}

void MarkdownBlock::setPlaceholder(QString placeholder) {
	m_placeholder = placeholder;
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
			std::max((qreal)0.0, point.x() - layoutPosition.x()),
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
				int p = line.xToCursor(pos.x());
				out->block = this;
				out->line = idx;
				out->position = p;
				return true;
			}
		}
	}

	return false;
}

bool MarkdownBlock::cursorBelow(
	MarkdownCursor cur,
	MarkdownCursor *result
) {
	if (cur.block != this)
		return false;

	QList<text::Line> *lines = m_par->getLines();
	int idx = cur.line;
	if (idx == -1 || idx >= lines->size())
		return false;

	QTextLayout *layout = m_layouts[cur.line];
	if (layout->isValidCursorPosition(cur.position) == false) {
		cur.position = 0;
	}

	QTextLine textLine = layout->lineForTextPosition(cur.position);
	if (textLine.isValid() == false)
		return false;

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
		result->line = idx + 1;
		result->position = pos;
		return true;
	}

	return false;
}

bool MarkdownBlock::cursorAbove(
	MarkdownCursor cur,
	MarkdownCursor *result
) {
	if (cur.block != this || cur.line == -1)
		return false;

	QList<text::Line> *lines = m_par->getLines();
	if (cur.line == -1 || cur.line >= lines->size())
		return false;

	int idx = cur.line;
	QTextLayout *layout = m_layouts[idx];
	if (layout->isValidCursorPosition(cur.position) == false) {
		// Invalid or empty line. Try the previous line.
		if (idx > 0) {
			// We have prev layout to move to.
			// Calculate the new cursor position in that layout for the same X.
			QTextLayout *nextLayout = m_layouts[idx - 1];
			QTextLine nextLine = nextLayout->lineAt(nextLayout->lineCount() - 1);
			result->line = idx - 1;
			result->position = 0;
			return true;
		}
	}

	QTextLine textLine = layout->lineForTextPosition(cur.position);
	if (textLine.isValid() == false)
		return false;

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
		result->line = idx - 1;
		result->position = pos;
		return true;
	}

	return false;
}

qreal MarkdownBlock::xAtCursor(MarkdownCursor cur) {
	if (cur.block != this || cur.line == -1 || cur.line >= m_layouts.size())
		return 0;

	QTextLayout *layout = m_layouts[cur.line];
	if (layout->isValidCursorPosition(cur.position) == false)
		return 0;

	QTextLine textLine = layout->lineForTextPosition(cur.position);
	if (textLine.isValid() == false)
		return false;

	qreal x = textLine.cursorToX(cur.position);

	return x;
}

MarkdownCursor MarkdownBlock::firstCursorAtX(qreal x) {
	MarkdownCursor cursor(nullptr, -1, 0);

	if (m_layouts.size() == 0)
		return cursor;

	QTextLayout *layout = m_layouts[0];
	QTextLine line = layout->lineAt(0);
	if (!line.isValid())
		return cursor;

	int pos = line.xToCursor(x);

	cursor.block = this;
	cursor.line = 0;
	cursor.position = pos;

	return cursor;
}

MarkdownCursor MarkdownBlock::lastCursorAtX(qreal x) {
	MarkdownCursor cursor(nullptr, -1, 0);

	if (m_layouts.size() == 0)
		return cursor;

	int lastLineIdx = m_layouts.size() - 1;
	QTextLayout *layout = m_layouts[lastLineIdx];
	QTextLine line = layout->lineAt(layout->lineCount() - 1);
	if (!line.isValid())
		return cursor;

	int pos = line.xToCursor(x);

	cursor.block = this;
	cursor.line = lastLineIdx;
	cursor.position = pos;

	return cursor;
}

bool MarkdownBlock::endOfBlock(MarkdownCursor *cursor) {
	if (m_layouts.size() == 0)
		return false;

	QList<text::Line> *lines = m_par->getLines();
	if (lines->size() == 0)
		return false;

	Line line = lines->at(lines->size() - 1);
	int pos = line.text.length();

	cursor->block = this;
	cursor->line = lines->size() - 1;
	cursor->position = pos;

	return true;
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
		qreal lineY = 0, offset = 0;
		const QTextLayout *item = m_layouts.at(i);

		text::Line *parLine = &((*m_par->getLines())[i]);
		if ((type == text::BulletList || type == text::NumberList) && parLine->level > 0) {
			offset += parLine->level * listLevelOffset;
		}

		QTextLayout *layout = (QTextLayout*)item;
		layout->setPosition(QPoint(start + offset, height));
		layout->beginLayout();

		while (true) {
			QTextLine line = layout->createLine();
			if (!line.isValid())
				break;

			line.setLineWidth(lineWidth - offset);
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
	if (m_par == nullptr)
		return;

	QMargins margins = contentsMargins();
	QMargins formatMargins = QMargins(0, 0, 0, 0);
	text::ParagraphType type = m_par->getType();
	QList<text::Line> *lines = m_par->getLines();

	MarkdownCursor *cursor = nullptr;
	if (m_provider != nullptr)
		cursor = m_provider->currentCursor();

	QPainter painter(this);

	QFontMetrics fontMetrics = QFontMetrics(m_style->textEditFont());
	if (type == text::Code) {
		painter.setFont(m_style->codeFont());
		fontMetrics = QFontMetrics(m_style->codeFont());
		formatMargins = codeMargins;
	} else if (type == text::BulletList || type == text::NumberList) {
		painter.setFont(m_style->textEditFont());
		formatMargins = listMargins;
	} else {
		painter.setFont(m_style->textEditFont());
	}

	if (
		!m_placeholder.isEmpty() &&
		m_provider != nullptr &&
		m_provider->isDocumentEmpty()
	) {
		// Draw placeholder.
		QColor placeholderColor = m_style->textEditColor();
		placeholderColor.setAlpha(128);

		painter.setPen(placeholderColor);
		painter.drawText(
			QPointF(margins.left(), margins.top() + fontMetrics.ascent()),
			m_placeholder
		);
	}

	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(m_style->textEditColor());

	int lineSpacing = fontMetrics.lineSpacing();
	int lineWidth = size().width()
		- margins.left() - margins.right()
		- formatMargins.left() - formatMargins.right();
	qreal y = margins.top() + formatMargins.top();

	// List enumeration by level. We need this to maintain enumeration on the
	// same level.
	QStack<unsigned int> enumStack;
	unsigned int prevLevel = 0, levelCount = 0;

	for (qsizetype i = 0; i < m_layouts.size(); ++i) {
		bool hollowPoint = false;
		qreal offset = 0;
		const QTextLayout *item = m_layouts.at(i);
		QTextLayout *layout = (QTextLayout*)item;

		text::Line *parLine = &((*m_par->getLines())[i]);
		offset = parLine->level * listLevelOffset;
		hollowPoint = (parLine->level + 2) % 2 == 1;

		if (parLine->level == prevLevel) {
			levelCount += 1;
		} else if (parLine->level > prevLevel) {
			while (parLine->level > enumStack.size()) {
				enumStack.push(levelCount);
				levelCount = 0;
			}
			levelCount = 1;
		} else {
			while (parLine->level < enumStack.size())
				levelCount = enumStack.pop() + 1;
		}
		prevLevel = parLine->level;

		// Draw list thingy.
		if (type == text::BulletList) {
			if (hollowPoint)
				painter.setBrush(Qt::NoBrush);
			else
				painter.setBrush(m_style->textEditColor());

			painter.drawEllipse(
				QRectF(
					(formatMargins.left() - fontMetrics.xHeight()) / 2 + offset,
					y + (fontMetrics.ascent() - fontMetrics.xHeight()) - 1,
					fontMetrics.xHeight(),
					fontMetrics.xHeight()
				)
			);
		} else if (type == text::NumberList) {
			QString number = QString("%1.").arg(levelCount);
			QRect numberRect = fontMetrics.boundingRect(number);
			qreal xPos = (formatMargins.left() - fontMetrics.xHeight()) / 2 + offset;

			painter.drawText(
				QPointF(
					xPos,
					y + fontMetrics.lineSpacing() - fontMetrics.descent()
				),
				number
			);

			// TODO: We don't take into account the space required for the number,
			// which can lead in overlap between line text and numbering, but only
			// in case of very big lists of 100+ items. Ideally the layou should be
			// adjusted for this case to move further to the right.
		}

		// Apply current selection.
		QList<QTextLayout::FormatRange> selections;
		if (m_provider != nullptr) {
			if (
				auto sel = m_provider->selectionInLine(this, &((*lines)[i]));
				sel.length > 0
			) {
				selections = { sel };
			}
		}

		// Draw the line/paragraph.
		painter.setBrush(m_style->textEditColor());
		layout->draw(&painter, QPointF(0, 0), selections);

		// Draw cursor.
		if (
			cursor != nullptr &&
			cursor->block == this &&
			cursor->line == i
		) {
			layout->drawCursor(&painter, QPointF(0, 0), cursor->position, 1);
		}

		// Adjust Y for next layout/line.
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

	// TODO: don't relayout everything?
	if (from.line != -1 && from.block == this && from.line < lines->size()) {
		setParagraph(m_par);
	} else if (to.line != -1 && to.block == this && to.line < lines->size()) {
		setParagraph(m_par);
	}
}

QLine MarkdownBlock::lineForCursor(MarkdownCursor cursor) {
	int idx = 0;
	QRect geo = geometry();

	if (parent() == nullptr) {
		return QLine(0, 0, 0, 0);
	}

	if (cursor.block != this || cursor.line == -1) {
		return QLine(0, 0, 0, 0);
	}

	QFontMetrics fontMetrics = QFontMetrics(m_style->textEditFont());
	QMargins margins = contentsMargins();

	QMargins formatMargins = QMargins(0, 0, 0, 0);
	if (m_par->getType() == text::Code) {
		fontMetrics = QFontMetrics(m_style->codeFont());
		formatMargins = codeMargins;
	} else if (m_par->getType() == text::BulletList || m_par->getType() == text::NumberList) {
		formatMargins = listMargins;
	}

	QList<text::Line> *lines = m_par->getLines();
	bool found = (cursor.line < lines->size() && cursor.line != -1);

	if (!found) {
		return QLine(0, 0, 0, 0);
	}

	QTextLayout *layout = m_layouts[cursor.line];
	QPointF pos = layout->position();

	if (layout->isValidCursorPosition(cursor.position) == false) {
		// Empty line. Return coordinates of the line.
		QTextLine line = layout->lineAt(0);
		if (line.isValid() == false)
			return QLine(0, 0, 0, 0);

		qreal height = fontMetrics.lineSpacing();
		return QLine(
			geo.x(),
			geo.y() + pos.y() + margins.top() + formatMargins.top() + line.y() + height * 1.5,
			geo.x(),
			geo.y() + pos.y() + margins.top() + formatMargins.top() + line.y() + height * 2.0
		);
	}

	// Non-empty line. Calculate the position of the cursor.
	QTextLine line = layout->lineForTextPosition(cursor.position);
	if (line.isValid() == false) {

		qreal height = fontMetrics.lineSpacing();
		return QLine(
			geo.x(),
			geo.y() + pos.y() + margins.top() + formatMargins.top() + line.y() + height * 1.5,
			geo.x(),
			geo.y() + pos.y() + margins.top() + formatMargins.top() + line.y() + height * 2.0
		);
	}

	qreal x = line.cursorToX(cursor.position);
	qreal height = line.height();

	return QLine(
		QPoint(geo.x() + x, geo.y() + pos.y() + line.y() - height * 0.5),
		QPoint(geo.x() + x, geo.y() + pos.y() + line.y() + height * 1.5)
	);
}

QPoint MarkdownBlock::pointAtCursor(MarkdownCursor cursor) {
	if (cursor.block != this)
		return QPoint(0, 0);

	QLine line = lineForCursor(cursor);
	return line.p2();
}

// Helpers.

QList<QTextLayout::FormatRange> MarkdownBlock::convertRanges(
	QList<FormatRange> from
) {
	QList<QTextLayout::FormatRange> result;

	QTextCharFormat defaultFormat;
	defaultFormat.setFont(m_style->textEditFont());

	for (auto fmt: from) {
		QTextCharFormat combined = defaultFormat;

		// Apply previous formatting. This only works for completely
		// nested formats, like <it>my <bold>text</bold> here</it>.
		// As far as I know Markdown doesn't really support intersecting
		// formats.
		for (auto prev: from) {
			if (prev.from <= fmt.from && prev.to >= fmt.to) {
				combined = qtFormat(prev, m_style, combined);
			}
		}

		result.push_back(
			QTextLayout::FormatRange{
				.start = fmt.from,
				.length = fmt.to - fmt.from,
				.format = qtFormat(fmt, m_style, combined)
			}
		);
	}

	return result;
}

inline qreal pxToPt(qreal px, qreal dpi) {
	return px*72.0/dpi;
}

QTextCharFormat MarkdownBlock::qtFormat(
	text::FormatRange range,
	Style *style,
	QTextCharFormat fmt
) {
	text::BlockFormat format = range.format;
	double dpi = QGuiApplication::primaryScreen()->physicalDotsPerInch() / devicePixelRatio();

	switch (format) {
		case text::Heading1:
			fmt.setFontPointSize(pxToPt(style->textEditFont().pixelSize() * 1.6, dpi));
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading2:
			fmt.setFontPointSize(pxToPt(style->textEditFont().pixelSize() * 1.5, dpi));
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading3:
			fmt.setFontPointSize(pxToPt(style->textEditFont().pixelSize() * 1.4, dpi));
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading4:
			fmt.setFontPointSize(pxToPt(style->textEditFont().pixelSize() * 1.3, dpi));
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading5:
			fmt.setFontPointSize(pxToPt(style->textEditFont().pixelSize() * 1.2, dpi));
			fmt.setFontWeight(QFont::Bold);
			break;
		case text::Heading6:
			fmt.setFontPointSize(pxToPt(style->textEditFont().pixelSize() * 1.1, dpi));
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

