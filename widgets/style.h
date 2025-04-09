#ifndef H_WIDGET_STYLE
#define H_WIDGET_STYLE

#include <QFont>
#include <QColor>
#include <QObject>

class Style: public QObject {
	Q_OBJECT

public:
	// Constructor.
	Style(
		QColor background,
		QColor nodeBackground,
		float borderWidth,
		QColor borderColor,
		float hoverBorderWidth,
		QColor hoverBorderColor,
		QFont font,
		QColor activeAnchorColor,
		QColor textColor,
		QColor hoverBackground,
		int scrollWidth,
		QColor anchorHighlight,
		QFont textEditFont,
		QColor textEditColor,
		QFont codeFont,
		QColor codeBackground,
		QColor linkColor,
		QFont iconFont
	);
	// Default style.
	static Style& defaultStyle();
	// Background color.
	const QColor background() const;
	void setBackground(QColor);
	// Node color.
	const QColor nodeBackground() const;
	void setNodeBackground(QColor);
	// Border settings.
	const float borderWidth() const;
	void setBorderWidth(float);
	const QColor borderColor() const;
	void setBorderColor(QColor);
	// Border on hover.
	const float hoverBorderWidth() const;
	void setHoverBorderWidth(float);
	const QColor hoverBorderColor() const;
	void setHoverBorderColor(QColor);
	// Background on hover.
	const QColor hoverBackground() const;
	void setHoverBackground(QColor);
	// Main font.
	const QFont font() const;
	void setFont(QFont);
	// Text Color.
	const QColor textColor() const;
	void setTextColor(QColor);
	// Anchor colors.
	const QColor activeAnchorColor() const;
	void setActiveAnchorColor(QColor);
	// Scroll bar width.
	const int scrollWidth() const;
	void setScrollWidth(int);
	// Anchor highlight.
	const QColor anchorHighlight() const;
	void setAnchorHighlight(QColor);
	// Text font.
	const QFont textEditFont() const;
	void setTextEditFont(QFont);
	const QColor textEditColor() const;
	void setTextEditColor(QColor);
	// Code font.
	const QFont codeFont() const;
	void setCodeFont(QFont);
	// Code back.
	const QColor codeBackground() const;
	void setCodeBackground(QColor color);
	// Links.
	const QColor linkColor() const;
	void setLinkColor(QColor);
	// Menu style.
	QString menuStyle();
	// Icons.
	const QFont iconFont() const;
	void setIconFont(QFont);
	// Brain list style.
	QString brainListButtonStyle(QString, QColor);
	QString brainItemButtonStyle();

signals:
	void styleChanged(Style*);

private:
	QColor m_background;
	QColor m_nodeBackground;
	QColor m_borderColor;
	QColor m_hoverBorderColor;
	QColor m_activeAnchorColor;
	QColor m_textColor;
	QColor m_hoverBackground;
	QColor m_anchorHighlight;
	QFont m_font;
	QFont m_textEditFont;
	QColor m_textEditColor;
	QFont m_codeFont;
	float m_borderWidth;
	float m_hoverBorderWidth;
	int m_scrollWidth;
	QColor m_codeBackground;
	QColor m_linkColor;
	QFont m_iconFont;
};

#endif

