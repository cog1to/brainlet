#include <QString>
#include <QRegularExpression>
#include <QList>
#include <QFont>
#include <QTextCharFormat>

#include "widgets/style.h"
#include "model/new_text_model.h"

// Utils.

namespace text_utils {
	inline int blockStartOffset(text::BlockFormat format) {
		switch (format) {
			case text::Heading1:
				return 2;
			case text::Heading2:
				return 3;
			case text::Heading3:
				return 4;
			case text::Heading4:
				return 5;
			case text::Heading5:
				return 6;
			case text::Heading6:
				return 7;
			case text::Italic:
				return 1;
			case text::Bold:
				return 2;
			case text::BoldItalic:
				return 3;
			case text::CodeSpan:
				return 1;
			case text::Link: // Links are handled separately.
				return -1;
			case text::NodeLink: // Links are handled separately.
				return -1;
			case text::PlainLink:
				return 0;
			case text::Escape:
				return 1;
		}

		assert(false); // Should be unreachable.
		return 0;
	}

	inline int blockEndOffset(text::BlockFormat format) {
		switch (format) {
			case text::Heading1:
			case text::Heading2:
			case text::Heading3:
			case text::Heading4:
			case text::Heading5:
			case text::Heading6:
			case text::Escape:
			case text::PlainLink:
				return 0;
			case text::Italic:
				return 1;
			case text::Bold:
				return 2;
			case text::BoldItalic:
				return 3;
			case text::CodeSpan:
				return 1;
			case text::Link: // Links are handled separately.
				return -1;
			case text::NodeLink: // Links are handled separately.
				return -1;
		}

		assert(false); // Should be unreachable.
		return 0;
	}
}

text::LinkFormat::LinkFormat(QString txt) : target(txt) {}

text::FormatRange::FormatRange(
	int _from,
	int _to,
	text::BlockFormat _format,
	text::LinkFormat _link
) : from(_from), to(_to), format(_format), link(_link) {}

int text::FormatRange::startOffset() const {
	if (format == text::Link || format == text::NodeLink) {
		return 1;
	} else {
		return text_utils::blockStartOffset(format);
	}
}

int text::FormatRange::endOffset() const {
	if (format == text::Link || format == text::NodeLink) {
		return 3 + link.target.size();
	} else {
		return text_utils::blockEndOffset(format);
	}
}


// Line parsing.

text::Line::Line(QString& input, bool preformatted) {
	setText(input, preformatted);
}

void text::Line::parseLinks(QString *input) {
	QRegularExpression expr("\\[(.+?)\\]\\(((.+?)://(.+?))\\)");
	QRegularExpressionMatch match = expr.match(*input);
	int offset;

	while (match.hasMatch()) {
		// Calcualate offset in the folded string by counting offsets applied
		// from previous folding operations.
		offset = 0;
		for (auto& format: foldedFormats) {
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

		text::BlockFormat linkType = BlockFormat::Link;
		if (match.captured(3) == "node")
			linkType = BlockFormat::NodeLink;

		// Folded text highlights the link title.
		FormatRange foldedRange = FormatRange(
			match.capturedStart() - offset,
			match.capturedStart() - offset + match.captured(1).size(),
			linkType,
			text::LinkFormat(match.captured(2))
		);

		// Unfolded text highlights the link target and title brackets.
		formats.push_back(
			FormatRange(
				match.capturedStart() + 3 + match.captured(1).size(),
				match.capturedEnd() - 1,
				linkType,
				text::LinkFormat(match.captured(2))
			)
		);
		// Brackets highlight.
		formats.push_back(
			FormatRange(
				match.capturedStart(),
				match.capturedStart() + 1,
				text::BlockFormat::Bold
			)
		);
		formats.push_back(
			FormatRange(
				match.capturedStart() + 1 + match.captured(1).size(),
				match.capturedStart() + 1 + match.captured(1).size() + 2,
				text::BlockFormat::Bold
			)
		);
		formats.push_back(
			FormatRange(
				match.capturedEnd() - 1,
				match.capturedEnd(),
				text::BlockFormat::Bold
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

void text::Line::parseSimpleLinks(QString *input) {
	// TODO: Hacky URL matching. Feels like covering the full URL spec and
	// detecting link text within normal text properly is almost impossible...
	QRegularExpression expr("(^| )(\\w+?://.+?)($| |[\\.,;!?\\-] )");
	QRegularExpressionMatch match = expr.match(*input);
	int offset;

	while (match.hasMatch()) {
		// Get offset from formats before.
		offset = 0;
		for (auto& format: foldedFormats) {
			if (format.from < match.capturedStart())
				offset += format.startOffset();
			if (format.to < match.capturedStart())
				offset += format.endOffset();
		}

		FormatRange foldedRange = FormatRange(
			match.capturedStart() - offset + match.captured(1).size(),
			match.capturedStart() - offset + match.captured(1).size() + match.captured(2).size(),
			text::BlockFormat::PlainLink,
			text::LinkFormat(match.captured(2))
		);

		formats.push_back(
			FormatRange(
				match.capturedStart() + match.captured(1).size(),
				match.capturedStart() + match.captured(1).size() + match.captured(2).size(),
				text::BlockFormat::PlainLink,
				text::LinkFormat(match.captured(2))
			)
		);

		foldedFormats.push_back(foldedRange);

		match = expr.match(*input, match.capturedEnd());
	}
}

void text::Line::apply(
	QString *input,
	text::BlockFormat fmt,
	QRegularExpression expr
) {
	int offset = 0;
	int prefix = text_utils::blockStartOffset(fmt);
	int suffix = text_utils::blockEndOffset(fmt);
	QRegularExpressionMatch match = expr.match(*input);

	while (match.hasMatch()) {
		// Get offset from formats before.
		offset = 0;
		for (auto& format: foldedFormats) {
			if (format.from < match.capturedStart())
				offset += format.startOffset();
			if (format.to < match.capturedStart())
				offset += format.endOffset();
		}

		folded
			.remove(match.capturedStart() - offset + match.captured(1).size(), prefix);
		folded
			.remove(
				match.capturedEnd()
					- match.captured(2).size()
					- offset - prefix - suffix,
				suffix
			);

		FormatRange foldedRange = FormatRange(
			match.capturedStart() - offset + match.captured(1).size(),
			match.capturedEnd() - offset - prefix - suffix - match.captured(2).size(),
			fmt
		);

		formats.push_back(
			FormatRange(
				match.capturedStart() + match.captured(1).size(),
				match.capturedEnd() - match.captured(2).size(),
				fmt
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

void text::Line::setText(QString& input, bool preformatted) {
	text = input;
	folded = input;

	formats.clear();
	foldedFormats.clear();

	if (preformatted) {
		return;
	}

	// Headings.
	if (input.startsWith("# ")) {
		folded = input.right(folded.size() - 2);
		foldedFormats.push_back(FormatRange(0, folded.size(), text::BlockFormat::Heading1));
		formats.push_back(FormatRange(0, input.size(), text::BlockFormat::Heading1));
	} else if (input.startsWith("## ")) {
		folded = input.right(folded.size() - 3);
		foldedFormats.push_back(FormatRange(0, folded.size(), text::BlockFormat::Heading2));
		formats.push_back(FormatRange(0, input.size(), text::BlockFormat::Heading2));
	} else if (input.startsWith("### ")) {
		folded = input.right(folded.size() - 4);
		foldedFormats.push_back(FormatRange(0, folded.size(), text::BlockFormat::Heading3));
		formats.push_back(FormatRange(0, input.size(), text::BlockFormat::Heading3));
	} else if (input.startsWith("#### ")) {
		folded = input.right(folded.size() - 5);
		foldedFormats.push_back(FormatRange(0, folded.size(), text::BlockFormat::Heading4));
		formats.push_back(FormatRange(0, input.size(), text::BlockFormat::Heading4));
	} else if (input.startsWith("##### ")) {
		folded = input.right(folded.size() - 6);
		foldedFormats.push_back(FormatRange(0, folded.size(), text::BlockFormat::Heading5));
		formats.push_back(FormatRange(0, input.size(), text::BlockFormat::Heading5));
	} else if (input.startsWith("###### ")) {
		folded = input.right(folded.size() - 7);
		foldedFormats.push_back(FormatRange(0, folded.size(), text::BlockFormat::Heading6));
		formats.push_back(FormatRange(0, input.size(), text::BlockFormat::Heading6));
	}

	// Escaping.
	apply(
		&text,
		BlockFormat::Escape,
		QRegularExpression("()\\\\[\\*]()")
	);

	// Code.
	apply(
		&text,
		BlockFormat::CodeSpan,
		QRegularExpression("()`[^\n]*?`()")
	);

	// Bold italic.
	apply(
		&text,
		BlockFormat::BoldItalic,
		QRegularExpression("(^|[^\\*\\\\])\\*\\*\\*[^\n]*?\\*\\*\\*($|[^\\*])")
	);

	// Bold.
	apply(
		&text,
		BlockFormat::Bold,
		QRegularExpression("(^|[^\\*\\\\])\\*\\*[^\\*][^\n]*?\\*\\*($|[^\\*])")
	);

	// Italic.
	apply(
		&text,
		BlockFormat::Italic,
		QRegularExpression("(^|[^\\*\\\\])\\*[^\\*\n]+?\\*($|[^\\*])")
	);

	// Remaining Links.
	parseSimpleLinks(&text);

	// Links.
	parseLinks(&text);
}

// Paragraphs.

text::Paragraph::Paragraph(
	text::ParagraphType type, QList<text::Line> lines
) : m_type(type), m_lines(lines) {}

text::Paragraph::Paragraph(text::ParagraphType type, text::Line line) {
	m_type = type;
	m_lines = QList { line };
}

text::ParagraphType text::Paragraph::getType() {
	return m_type;
}

void text::Paragraph::setType(text::ParagraphType type) {
	m_type = type;
}

QList<text::Line> *text::Paragraph::getLines() {
	return &m_lines;
}

void text::Paragraph::setLine(int idx, text::Line line) {
	m_lines.replace(idx, line);
}	

void text::Paragraph::setLines(QList<text::Line> lines) {
	m_lines = lines;
}

int text::Paragraph::indexOfLine(text::Line* line) {
	for (int idx = 0; idx < m_lines.size(); idx++) {
		if (&(m_lines[idx]) == line)
			return idx;
	}

	return -1;
}

// Text model.

text::TextModel::TextModel() {}

text::TextModel::TextModel(QList<Paragraph> pars) {
	m_data = pars;
}

text::TextModel::TextModel(QStringList data) {
	static QRegularExpression bulletExp("^([\\-\\*\\+]) ");
	static QRegularExpression numberExp("^([0-9]+\\.) ");

	int emptyCount = 0;
	QList<text::Line> currentLines;
	text::ParagraphType type = text::Text;
	QStringList content;

	for (auto it = data.begin(); it != data.end(); it++) {
		QString& line = *it;

		if (line == "```") {
			if (type != text::Code) {
				// Look ahead for the closing sequence of the code block.
				bool closureFound = false;
				for (auto next = it + 1; next != data.end(); next++) {
					if (*next == "```") {
						closureFound = true;
						break;
					}
				}

				// If closure is found, apply code formatting. Otherwise ignore it.
				if (closureFound) {
					type = text::Code;
				}
			} else {
				type = text::Text;
				// Save the code block as a paragraph.
				m_data.push_back(text::Paragraph(text::Code, currentLines));
				currentLines.clear();
			}
			continue;
		}

		if (type == text::Code) {
			currentLines.push_back(Line(line, true));
			continue;
		}

		if (line.isEmpty()) {
			if (type != text::Text) {
				m_data.push_back(
					text::Paragraph(type, currentLines)
				);
				currentLines.clear();
				type = text::Text;
			} else if (content.size() > 0) {
				QString paragraph = content.join(" ");
				m_data.push_back(
					text::Paragraph(text::Text, text::Line(paragraph, false))
				);
				content = QStringList();
			} else if (emptyCount == 2) {
				emptyCount = 0;
				QString str = "";
				m_data.push_back(
					text::Paragraph(text::Text, text::Line(str, false))
				);
			}
			emptyCount += 1;
		} else {
			if (
				QRegularExpressionMatch match = bulletExp.match(line);
				match.hasMatch()
			) {
				QString text = line.right(line.length() - match.capturedEnd());
				if (type != text::BulletList && currentLines.size() > 0) {
					m_data.push_back(
						text::Paragraph(type, currentLines)
					);
					currentLines.clear();
				}

				type = text::BulletList;
				currentLines.push_back(text::Line(text, false));
			} else if (
				QRegularExpressionMatch match = numberExp.match(line);
				match.hasMatch()
			) {
				QString text = line.right(line.length() - match.capturedEnd());
				if (type != text::NumberList && currentLines.size() > 0) {
					m_data.push_back(
						text::Paragraph(type, currentLines)
					);
					currentLines.clear();
					type = text::NumberList;
				}

				type = text::NumberList;
				currentLines.push_back(text::Line(text, false));
			} else {
				content.push_back(line);
			}

			emptyCount = 0;
		}
	}

	// Add last paragraph.
	if (currentLines.size() > 0) {
		m_data.push_back(
			text::Paragraph(type, currentLines)
		);
	} else if (content.size() > 0 && type == text::Text) {
		QString paragraph = content.join(" ");
		m_data.push_back(
			text::Paragraph(text::Text, text::Line(paragraph, false))
		);
	}
}

QList<text::Paragraph> *text::TextModel::paragraphs() {
	return &m_data;
}

const QList<text::Paragraph> *text::TextModel::const_paragraphs() const {
	return &m_data;
}

void text::TextModel::setParagraphs(QList<text::Paragraph> data) {
	m_data = data;
}

QString text::TextModel::text() {
	QStringList result;

	for (auto par = m_data.begin(); par != m_data.end(); par++) {
		QList<text::Line> *lines = (*par).getLines();
		QStringList parLines;
		text::ParagraphType type = (*par).getType();
		
		// Wrap code in ```
		if (type == text::Code)
			parLines.push_back("```");

		// Copy each line.
		for (auto line = lines->begin(); line != lines->end(); line++) {
			// Add prefix for lists.
			QString prefix = "";
			if (type == text::NumberList)
				prefix = "1. ";
			else if (type == text::BulletList)
				prefix = "- ";

			parLines.push_back(prefix + (*line).text);
		}

		// Wrap code in ```
		if (type == text::Code)
			parLines.push_back("```");

		// Join lines.
		result.push_back(parLines.join("\n"));
	}

	// Join paragraphs.
	return result.join("\n\n");
}

