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
	Heading4,
	Heading5,
	Heading6,
	Italic,
	Bold,
	BoldItalic,
	Code,
	CodeBlock
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

enum ListType {
	ListNone,
	ListBullet,
	ListNumeric
};

struct ListItem {
public:
	ListType listType;
	int level;
	QString value;
	// Constructor.
	ListItem(ListType lt = ListNone, int l = 0, QString v = "")
		: listType(lt), level(l), value(v) {};
};

struct Line {
public:
	QString text;
	QString folded;
	std::vector<FormatRange> formats;
	std::vector<FormatRange> foldedFormats;
	ListItem list;
	bool isCodeBlock = false;
	// Constructors
	Line(QString&);
	Line(QString&, std::vector<FormatRange>);
	static Line codeLine(QString&);
	// Properties.
	const bool isListItem() const { return list.listType != ListNone; };

private:
	void apply(QString *input, BlockFormat format, QRegularExpression expr, int size);
};

class TextModel {
public:
	TextModel();
	TextModel(QStringList);
	// Data.
	std::vector<Line> *lines();

private:
	std::vector<Line> m_data;
};

#endif
