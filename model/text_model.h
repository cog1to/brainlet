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
	CodeBlock,
	Link,
	PlainLink,
	NodeLink
};

struct LinkFormat {
	QString target;
	LinkFormat(QString str = ""): target(str) {};
};

struct FormatRange {
	int from, to;
	BlockFormat format;
	LinkFormat link;
	// Constructor.
	FormatRange(int _from, int _to, BlockFormat _format, LinkFormat _link = LinkFormat())
		: from(_from), to(_to), format(_format), link(_link) {};
	// Convert to QTextCharFormat
	QTextCharFormat qtFormat(Style*, QTextCharFormat);
	// Symbolic offset due to formatting symbols.
	int startOffset() const;
	int endOffset() const;
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
	Line(QString&, ListItem);
	static Line codeLine(QString&);
	// Properties.
	const bool isListItem() const { return list.listType != ListNone; };
	// Modificators.
	void setText(QString&);

private:
	void apply(QString *input, BlockFormat format, QRegularExpression expr, int size);
	void parseLinks(QString *input);
	void parseSimpleLinks(QString *input);
};

class TextModel {
public:
	TextModel();
	TextModel(QStringList);
	// Data.
	std::vector<Line> *lines();
	const std::vector<Line> *const_lines() const;
	void setLines(std::vector<Line>);

private:
	std::vector<Line> m_data;
};

#endif
