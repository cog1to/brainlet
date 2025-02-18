#ifndef H_TEXT_MODEL
#define H_TEXT_MODEL

#include <QString>
#include <QStringList>
#include <QTextCharFormat>

#include "widgets/style.h"

enum BlockFormat {
	Heading1,
	Heading2,
	Heading3,
	Italic,
	Bold,
	BoldItalic
};

struct FormatRange {
	int from, to;
	BlockFormat format;
	// Constructor.
	FormatRange(int _from, int _to, BlockFormat _format)
		: from(_from), to(_to), format(_format) {};
	// Convert to QTextCharFormat
	QTextCharFormat qtFormat(Style*, QTextCharFormat);
	// Symbolic offset due to formatting symbols.
	int startOffset();
	int endOffset();
};

struct Line {
	QString text;
	QString folded;
	std::vector<FormatRange> formats;
	std::vector<FormatRange> foldedFormats;
	// Constructor
	Line(QString&);

private:
	void apply(QString *input, BlockFormat format, QRegularExpression expr, int size);
};

class TextModel {
public:
	TextModel();
	TextModel(std::vector<QString>);
	TextModel(QStringList);
	// Data.
	std::vector<Line> lines();
	QString folded();

private:
	std::vector<Line> m_data;
};

#endif
