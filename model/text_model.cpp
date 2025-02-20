#include <QString>
#include <QRegularExpression>

#include "model/text_model.h"

// Utils

inline int blockStartOffset(BlockFormat format) {
	switch (format) {
		case Heading1:
			return 2;
		case Heading2:
			return 3;
		case Heading3:
			return 4;
		case Heading4:
			return 5;
		case Heading5:
			return 6;
		case Heading6:
			return 7;
		case Italic:
			return 1;
		case Bold:
			return 2;
		case BoldItalic:
			return 3;
		case Code:
			return 1;
		case CodeBlock:
			return 0;
		case Link: // Links are handled separately.
			return -1;
		case PlainLink:
			return 0;
	}

	assert(false); // Should be unreachable.
	return 0;
}

inline int blockEndOffset(BlockFormat format) {
	switch (format) {
		case Heading1:
		case Heading2:
		case Heading3:
		case Heading4:
		case Heading5:
		case Heading6:
			return 0;
		case Italic:
			return 1;
		case Bold:
			return 2;
		case BoldItalic:
			return 3;
		case Code:
			return 1;
		case CodeBlock:
			return 0;
		case Link: // Links are handled separately.
			return -1;
		case PlainLink:
			return 0;
	}

	assert(false); // Should be unreachable.
	return 0;
}

// Ranges

QTextCharFormat FormatRange::qtFormat(Style *style, QTextCharFormat fmt) {
	switch (format) {
		case Heading1:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 2.0);
			fmt.setFontWeight(QFont::Bold);
			break;
		case Heading2:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.8);
			fmt.setFontWeight(QFont::Bold);
			break;
		case Heading3:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.6);
			fmt.setFontWeight(QFont::Bold);
			break;
		case Heading4:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.4);
			fmt.setFontWeight(QFont::Bold);
			break;
		case Heading5:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.2);
			fmt.setFontWeight(QFont::Bold);
			break;
		case Heading6:
			fmt.setFontPointSize(style->textEditFont().pixelSize() * 1.1);
			fmt.setFontWeight(QFont::Bold);
			break;
		case Italic:
			fmt.setFontItalic(true);
			break;
		case Bold:
			fmt.setFontWeight(QFont::Bold);
			break;
		case BoldItalic:
			fmt.setFontItalic(true);
			fmt.setFontWeight(QFont::Bold);
			break;
		case Code:
			fmt.setBackground(style->codeBackground());
			fmt.setFont(style->codeFont());
			break;
		case CodeBlock:
			fmt.setBackground(style->codeBackground());
			fmt.setFont(style->codeFont());
			break;
		case Link:
		case PlainLink:
			fmt.setFontUnderline(true);
			fmt.setForeground(style->linkColor());
			fmt.setAnchorHref(link.target);
			fmt.setAnchor(true);
			break;
	}

	return fmt;
}

int FormatRange::startOffset() {
	if (format == Link) {
		return 1;
	} else {
		return blockStartOffset(format);
	}
}

int FormatRange::endOffset() {
	if (format == Link) {
		return 3 + link.target.size();
	} else {
		return blockEndOffset(format);
	}
}

// Lines

Line::Line(QString& input): text(input), folded(input) {
	int offset = 0;

	QRegularExpression enumeratedExp("^[0-9]+\\. ");
	QRegularExpression listExp("^[\\+\\*\\-] ");

	// Headings.
	if (input.startsWith("# ")) {
		folded = input.right(folded.size() - 2);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading1));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading1));
	} else if (input.startsWith("## ")) {
		folded = input.right(folded.size() - 3);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading2));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading2));
	} else if (input.startsWith("### ")) {
		folded = input.right(folded.size() - 4);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading3));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading3));
	} else if (input.startsWith("#### ")) {
		folded = input.right(folded.size() - 5);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading4));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading4));
	} else if (input.startsWith("##### ")) {
		folded = input.right(folded.size() - 6);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading5));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading5));
	} else if (input.startsWith("###### ")) {
		folded = input.right(folded.size() - 7);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading6));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading6));
	} else if (QRegularExpressionMatch match = listExp.match(input); match.hasMatch()) {
		// TODO: Sublist support.
		list = ListItem(ListBullet, 0);
		folded = input.right(input.size() - 2);
		text = input.right(input.size() - 2);
	} else if (QRegularExpressionMatch match = enumeratedExp.match(input); match.hasMatch()) {
		// TODO: Numeric list support and levels support.
		list = ListItem(ListNumeric, 0);
		folded = input.right(input.size() - match.captured(0).size());
		text = input.right(input.size() - match.captured(0).size());
	}

	// Code.
	apply(
		&text,
		BlockFormat::Code,
		QRegularExpression("()`[^\n]*?`()"),
		1		
	);

	// Bold italic.
	apply(
		&text,
		BlockFormat::BoldItalic,
		QRegularExpression("(^|[^\\*])\\*\\*\\*\\w[^\n]*?\\*\\*\\*($|[^\\*])"),
		3
	);

	// Bold.
	apply(
		&text,
		BlockFormat::Bold,
		QRegularExpression("(^|[^\\*])\\*\\*\\w[^\n]*?\\*\\*($|[^\\*])"),
		2
	);

	// Italic.
	apply(
		&text,
		BlockFormat::Italic,
		QRegularExpression("(^|[^\\*])\\*\\w[^\n]*?\\*($|[^\\*])"),
		1
	);

	// Links.
	parseLinks(&text);

	// Remaining Links.
	parseSimpleLinks(&text);
}

void Line::parseLinks(QString *input) {
	QRegularExpression expr("\\[(.+?)\\]\\((.+?://.+?)\\)");
	QRegularExpressionMatch match = expr.match(*input);
	int offset;
	
	while (match.hasMatch()) {
		// Get offset from formats before.
		offset = 0;
		for (auto& format: formats) {
			if (format.from < match.capturedStart())
				offset += format.startOffset();
			if (format.to < match.capturedStart())
				offset += format.endOffset();
		}
		
		folded
			.remove(match.capturedStart() - offset, 1)
			.remove(
				match.capturedEnd()
					- offset
					- 4
					- match.captured(2).size(),
				match.captured(2).size() + 3
			);

		// Folded text highlights the link title.
		FormatRange foldedRange = FormatRange(
			match.capturedStart() - offset,
			match.capturedStart() - offset + match.captured(1).size(),
			BlockFormat::Link,
			LinkFormat(match.captured(2))
		);

		// Unfolded text highlights the link target and title brackets.
		formats.push_back(
			FormatRange(
				match.capturedStart() + 3 + match.captured(1).size(),
				match.capturedEnd() - 1,
				BlockFormat::Link,
				LinkFormat(match.captured(2))
			)
		);
		// Brackets highlight.
		formats.push_back(
			FormatRange(
				match.capturedStart(),
				match.capturedStart() + 1,
				BlockFormat::Bold
			)
		);
		formats.push_back(
			FormatRange(
				match.capturedStart() + 1 + match.captured(1).size(),
				match.capturedStart() + 1 + match.captured(1).size() + 2,
				BlockFormat::Bold
			)
		);
		formats.push_back(
			FormatRange(
				match.capturedEnd() - 1,
				match.capturedEnd(),
				BlockFormat::Bold
			)
		);

		// Shift other formats.
		for (auto& format: foldedFormats) {
			if (format.from > foldedRange.from)
				format.from -= foldedRange.startOffset();
			if (format.to > foldedRange.from)
				format.to -= foldedRange.startOffset();
			if (format.from > foldedRange.to)
				format.from -= foldedRange.endOffset();
			if (format.to > foldedRange.to) {
				format.to -= foldedRange.endOffset();
			}
		}

		foldedFormats.push_back(foldedRange);

		match = expr.match(*input, match.capturedEnd());
	}
}

void Line::parseSimpleLinks(QString *input) {
	// TODO: Hacky URL matching. Feels like covering the full URL spec and
	// detecting link text within normal text properly is almost impossible...
	QRegularExpression expr("(^| )(\\w+?://.+?)($| |[\\.,;!?\\-] )");
	QRegularExpressionMatch match = expr.match(*input);
	int offset;
	
	while (match.hasMatch()) {
		// Get offset from formats before.
		offset = 0;
		for (auto& format: formats) {
			if (format.from < match.capturedStart())
				offset += format.startOffset();
			if (format.to < match.capturedStart())
				offset += format.endOffset();
		}
		
		FormatRange foldedRange = FormatRange(
			match.capturedStart() - offset + match.captured(1).size(),
			match.capturedStart() - offset + match.captured(1).size() + match.captured(2).size(),
			BlockFormat::PlainLink,
			LinkFormat(match.captured(2))
		);

		formats.push_back(
			FormatRange(
				match.capturedStart() + match.captured(1).size(),
				match.capturedStart() + match.captured(1).size() + match.captured(2).size(),
				BlockFormat::PlainLink,
				LinkFormat(match.captured(2))
			)
		);

		foldedFormats.push_back(foldedRange);

		match = expr.match(*input, match.capturedEnd());
	}
}

void Line::apply(
	QString *input,
	BlockFormat fmt,
	QRegularExpression expr,
	int size
) {
	int offset = 0;
	int prefix = blockStartOffset(fmt);
	int suffix = blockEndOffset(fmt);
	QRegularExpressionMatch match = expr.match(*input);

	while (match.hasMatch()) {
		// Get offset from formats before.
		offset = 0;
		for (auto& format: formats) {
			if (format.from < match.capturedStart())
				offset += format.startOffset();
			if (format.to < match.capturedStart())
				offset += format.endOffset();
		}

		folded
			.remove(match.capturedStart() - offset + match.captured(1).size(), prefix)
			.remove(match.capturedEnd() - offset - prefix - suffix - match.captured(2).size(), suffix);

		FormatRange foldedRange = FormatRange(
			match.capturedStart() - offset + match.captured(1).size(),
			match.capturedEnd() - offset - prefix - suffix - match.captured(2).size(),
			fmt
		);

		formats.push_back(FormatRange(
			match.capturedStart(),
			match.capturedEnd(),
			fmt
		));

		// Shift other formats.
		for (auto& format: foldedFormats) {
			if (format.from > foldedRange.from)
				format.from -= foldedRange.startOffset();
			if (format.to > foldedRange.from)
				format.to -= foldedRange.startOffset();
			if (format.from > foldedRange.to)
				format.from -= foldedRange.endOffset();
			if (format.to > foldedRange.to) {
				format.to -= foldedRange.endOffset();
			}
		}

		foldedFormats.push_back(foldedRange);

		match = expr.match(*input, match.capturedEnd());
	}
}

Line::Line(QString& input, std::vector<FormatRange> format)
	: isCodeBlock(true), text(input), folded(input), formats(format), foldedFormats(format) {}

Line Line::codeLine(QString& input) {
	std::vector<FormatRange> formats = {
		FormatRange(0, input.size(), BlockFormat::CodeBlock)
	};
	return Line(input, formats);
}

// Model

TextModel::TextModel() {}

TextModel::TextModel(QStringList data) {
	QStringList content;
	bool code = false;

	QRegularExpression listExp("^([\\-\\*\\+]|[0-9]+\\.) ");

	for (QString& line: data) {
		if (line == "```") {
			code = !code;
			continue;
		}

		if (code) {
			m_data.push_back(Line::codeLine(line));
			continue;
		}

		if (line.isEmpty()) {
			if (content.size() > 0) {
				QString paragraph = content.join(" ");
				m_data.push_back(Line(paragraph));
				content = QStringList();
			}
		} else {
			if (QRegularExpressionMatch match = listExp.match(line); match.hasMatch()) {
				// Add previous item.
				if (content.size() > 0) {
					QString paragraph = content.join(" ");
					m_data.push_back(Line(paragraph));
					content = QStringList();
				}
			}
			content.push_back(line);
		}
	}

	if (content.size() > 0) {
		QString paragraph = content.join(" ");
		m_data.push_back(Line(paragraph));
	}
}

std::vector<Line> *TextModel::lines() {
	return &m_data;
}

