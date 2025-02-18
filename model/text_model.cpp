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
		case Italic:
			return 1;
		case Bold:
			return 2;
		case BoldItalic:
			return 3;
	}

	assert(false); // Should be unreachable.
	return 0;
}

inline int blockEndOffset(BlockFormat format) {
	switch (format) {
		case Heading1:
		case Heading2:
		case Heading3:
			return 0;
		case Italic:
			return 1;
		case Bold:
			return 2;
		case BoldItalic:
			return 3;
	}

	assert(false); // Should be unreachable.
	return 0;
}

// Ranges

QTextCharFormat FormatRange::qtFormat(Style *style, QTextCharFormat fmt) {
	switch (format) {
		case Heading1:
			fmt.setFontPointSize(style->textFont().pixelSize() * 2.0);
			fmt.setFontWeight(QFont::Bold);
			break;
		case Heading2:
			fmt.setFontPointSize(style->textFont().pixelSize() * 1.8);
			fmt.setFontWeight(QFont::Bold);
			break;
		case Heading3:
			fmt.setFontPointSize(style->textFont().pixelSize() * 1.6);
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
	}

	return fmt;
}

int FormatRange::startOffset() {
	return blockStartOffset(format);
}

int FormatRange::endOffset() {
	return blockEndOffset(format);
}

// Lines

Line::Line(QString& input): text(input), folded(input) {
	int offset = 0;

	// Headings.
	if (folded.startsWith("# ")) {
		folded = folded.right(folded.size() - 2);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading1));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading1));
	} else if (folded.startsWith("## ")) {
		folded = folded.right(folded.size() - 3);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading2));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading2));
	} else if (folded.startsWith("### ")) {
		folded = folded.right(folded.size() - 4);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading3));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading3));
	}

	// Bold italic.
	apply(
		&text,
		BlockFormat::BoldItalic,
		QRegularExpression("(^|[^\\*])\\*\\*\\*[^\n]+?\\*\\*\\*($|[^\\*])"),
		3
	);

	// Bold.
	apply(
		&text,
		BlockFormat::Bold,
		QRegularExpression("(^|[^\\*])\\*\\*[^\\*][^\n]+?\\*\\*($|[^\\*])"),
		2
	);

	// Italic.
	apply(
		&text,
		BlockFormat::Italic,
		QRegularExpression("(^|[^\\*])\\*[^\\*\n]+?\\*($|[^\\*])"),
		1
	);
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

// Model

TextModel::TextModel() {}

TextModel::TextModel(std::vector<QString> data) {
	for (QString& line: data) {
		m_data.push_back(Line(line));
	}
}

TextModel::TextModel(QStringList data) {
	for (QString& line: data) {
		m_data.push_back(Line(line));
	}
}

std::vector<Line> TextModel::lines() {
	return m_data;
}

QString TextModel::folded() {
	if (m_data.size() == 0) {
		return "";
	}

	QStringList list;
	for (Line& line: m_data) {
		list << line.folded;
	}

	return list.join("\n");
}

