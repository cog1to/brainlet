#include <QString>
#include <QRegularExpression>

#include "model/text_model.h"

// Ranges

QTextCharFormat FormatRange::qtFormat(Style *style) {
	switch (format) {
		case Heading1:
			QTextCharFormat fmt;
			fmt.setFontPointSize(25);
			fmt.setFontWeight(QFont::Bold);
			return fmt;
	}

	assert(false); // Should be unreachable.
	return QTextCharFormat();
}

int FormatRange::startOffset() {
	switch (format) {
		case Heading1:
			return 2;
	}

	assert(false); // Should be unreachable.
	return 0;
}

int FormatRange::endOffset() {
	switch (format) {
		case Heading1:
			return 0;
	}

	assert(false); // Should be unreachable.
	return 0;
}

// Lines

Line::Line(QString& input): text(input), folded(input) {
	// Headings.
	if (folded.startsWith("# ")) {
		folded = folded.right(folded.size() - 2);
		foldedFormats.push_back(FormatRange(0, folded.size(), BlockFormat::Heading1));
		formats.push_back(FormatRange(0, input.size(), BlockFormat::Heading1));
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
