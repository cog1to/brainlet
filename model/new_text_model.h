#ifndef H_NEW_TEXT_MODEL
#define H_NEW_TEXT_MODEL

#include <QString>
#include <QList>
#include <QRegularExpression>

namespace text {
	// Paragraph tyles.
	enum ParagraphType {
		Text,
		Code,
		BulletList,
		NumberList
	};

	// Block formats.
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
		Link,
		PlainLink,
		NodeLink,
		CodeSpan,
		Escape
	};
	
	// Link metadata.
	struct LinkFormat {
		QString target;
		LinkFormat(QString str = "");
	};

	// Format data for a chunk of text.
	struct FormatRange {
		int from, to;
		BlockFormat format;
		LinkFormat link;
		// Constructor.
		FormatRange(
			int _from,
			int _to,
			BlockFormat _format,
			LinkFormat _link = LinkFormat()
		);
		// Symbolic offset due to formatting symbols.
		int startOffset() const;
		int endOffset() const;
	};

	// Single line model.
	class Line {
	public:
		Line(QString&, bool);
		// Properties.
		QString text;
		QList<FormatRange> formats;
		QString folded;
		QList<FormatRange> foldedFormats;
		// Modificators.
		void setText(QString&, bool);
		// Comparison.
		inline bool operator==(const Line rhs) const {
			return text == rhs.text;
		}
		inline bool operator!=(const Line rhs) const {
			return text != rhs.text;
		}
	private:
		void parseLinks(QString*);
		void parseSimpleLinks(QString*);
		void apply(QString*, BlockFormat, QRegularExpression);
	};

	// Paragraph model. Can consist of one or more lines.
	class Paragraph {
	public:
		Paragraph(ParagraphType, QList<Line>);
		Paragraph(ParagraphType, Line);
		// Properties.
		ParagraphType getType();
		void setType(ParagraphType);
		QList<Line> *getLines();
		void setLine(int, Line);
		void setLines(QList<Line>);
	private:
		ParagraphType m_type = Text;
		QList<Line> m_lines;
	};

	// Text model.
	class TextModel {
	public:
		TextModel();
		TextModel(QStringList);
		// Data.
		QList<Paragraph> *paragraphs();
		const QList<Paragraph> *const_paragraphs() const;
		void setParagraphs(QList<Paragraph>);

	private:
		QList<Paragraph> m_data;
	};
}

#endif

