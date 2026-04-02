#ifndef H_WIDGET_STYLE
#define H_WIDGET_STYLE

#include <QFont>
#include <QColor>
#include <QObject>

struct Fonts {
	QFont icon;
};

struct BrowserStyle {
	QFont browseFont;
	QFont historyFont;
	QColor text;
	QColor background;
	QColor node;
	QColor nodeHover;
	QColor nodeFocused;
	QColor border;
	QColor borderHover;
	QColor anchorActive;
	QColor anchorHighlight;
	float borderWidth;
	float hoverBorderWidth;
	int scrollWidth;
};

struct EditorStyle {
	QFont textFont;
	QFont monoFont;
	QColor text;
	QColor background;
	QColor codeBackground;
	QColor selectionText;
	QColor selectionBackground;
	QColor link;
	QColor nodeLink;
	QColor textHighlight;
};

struct BrainListStyle {
	QFont textFont;
	QColor text;
	QColor background;
	QColor foreground;
};

class Style: public QObject {
	Q_OBJECT

public:
	// Constructor.
	Style(
		Fonts,
		BrowserStyle,
		EditorStyle,
		BrainListStyle
	);
	// Default style.
	static Style& defaultStyle();
	// Component styles.
	Fonts fonts;
	BrowserStyle browser;
	EditorStyle editor;
	BrainListStyle brains;
	// Menu style.
	QString menuStyle();
	// Brain list style.
	QString brainListButtonStyle(QString, QColor);
	QString brainItemButtonStyle();

signals:
	void styleChanged(Style*);
};

#endif

