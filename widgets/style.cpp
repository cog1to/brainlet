#include <QFont>
#include <QColor>
#include <QObject>
#include <QFontDatabase>

#include "widgets/style.h"

Style::Style(
	Fonts _fonts,
	BrowserStyle _browser,
	EditorStyle _editor,
	BrainListStyle _brains
) {
	fonts = _fonts;
	browser = _browser;
	editor = _editor;
	brains = _brains;
}

Style& Style::defaultStyle() {
	QFontDatabase::addApplicationFont(":/fonts/fa-solid-900.ttf");
	QFontDatabase::addApplicationFont(":/fonts/noto-sans.ttf");
	QFontDatabase::addApplicationFont(":/fonts/noto-sans-mono.ttf");

	static QFont font = QFont("Noto Sans");
	font.setPixelSize(15);

	static QFont textFont = QFont("Noto Sans");
	textFont.setPixelSize(15);

	static QFont codeFont = QFont("Noto Sans Mono");
	codeFont.setPixelSize(15);

	static QFont iconFont = QFont("Font Awesome 6 Free");
	iconFont.setPixelSize(12);

	static QFont historyFont = QFont("Noto Sans Mono");
	historyFont.setPixelSize(12);

	static Fonts fonts = { .icon = iconFont };

	static BrowserStyle browser = {
		.browseFont = font,
		.historyFont = historyFont,
		.text = QColor(215, 221, 232, 255),
		.background = QColor(23, 43, 52, 255),
		.node = QColor(16, 31, 38, 128),
		.nodeHover = QColor(0, 0, 0, 192),
		.nodeFocused = QColor(255, 255, 255),
		.border = QColor(248, 144, 87, 255),
		.borderHover = QColor (248, 144, 87, 255),
		.anchorActive = QColor(228, 83, 75, 255),
		.anchorHighlight = QColor(255, 255, 255, 255),
		.borderWidth = 1.0,
		.hoverBorderWidth = 2.0,
		.scrollWidth = 12
	};

	static EditorStyle editor = {
		.textFont = textFont,
		.monoFont = codeFont,
		.text = QColor(215, 221, 232, 255),
		.background = QColor(23, 43, 52, 255),
		.codeBackground = QColor(43, 63, 72, 255),
		.selectionText = QColor(49, 79, 120),
		.selectionBackground = QColor(255, 255, 255),
		.link = QColor(162, 187, 219),
		.nodeLink = QColor(16, 31, 38, 128),
		.textHighlight = QColor(16, 31, 38, 128)
	};

	static BrainListStyle brains = {
		.textFont = textFont,
		.text = QColor(215, 221, 232, 255),
		.background = QColor(23, 43, 52, 255),
		.foreground = QColor (248, 144, 87, 255)
	};

	static Style style(
		fonts,
		browser,
		editor,
		brains
	);

	return style;
}

QString Style::menuStyle() {
	QString stylesheet = QString(
		"QMenu {\
			color: %1;\
			font: %2 %3px \"%4\";\
		}\
		QMenu::item {\
			padding-top: 5px;\
			padding-left: 15px;\
			padding-right: 15px;\
			padding-bottom: 9px;\
		}\
		QMenu::item:selected {\
			color: #ffffff;\
		}"
	).arg(browser.text.name(QColor::HexRgb))
	.arg(browser.browseFont.bold() ? "bold" : "")
	.arg(browser.browseFont.pixelSize())
	.arg(browser.browseFont.family());

	return stylesheet;
}

QString Style::brainListButtonStyle(QString align, QColor foreground) {
	QString stylesheet = QString(
		"QPushButton{\
			font: %1 %2px \"%3\";\
			color: %4;\
			border-width: 2px;\
			border-color: %5;\
			padding: 8px;\
			padding-left: 12px;\
			padding-right: 12px;\
			background-color: %6;\
			text-align: %10;\
			border-radius: 10px;\
			border-style: solid;\
			width: 194px;\
		}\
		QPushButton:hover{\
			background-color: %9;\
		}\
		QPushButton:hover:pressed{\
			border-color: %7;\
			background-color: %8;\
		}")
		.arg("bold")
		.arg(18)
		.arg(browser.browseFont.family())
		.arg(foreground.name(QColor::HexArgb))
		.arg(foreground.darker(200).name(QColor::HexArgb))
		.arg(browser.background.lighter(110).name(QColor::HexRgb))
		.arg(foreground.darker(300).name(QColor::HexArgb))
		.arg(browser.background.darker(110).name(QColor::HexRgb))
		.arg(browser.background.lighter(130).name(QColor::HexRgb))
		.arg(align);

	return stylesheet;
}

QString Style::brainItemButtonStyle() {
	return QString("QPushButton{\
			font: %1 %2px \"%3\";\
			color: %4;\
			border-width: 1px;\
			border-color: %5;\
			padding: 8px;\
			padding-left: 9px;\
			padding-right: 9px;\
			background-color: %6;\
			text-align: center;\
			border-radius: 10px;\
			border-style: solid;\
		}\
		QPushButton:hover{\
			color: %7;\
			border-color: %8;\
			background-color: %9;\
		}\
		QPushButton:hover:pressed{\
			color: %10;\
			border-color: %11;\
			background-color: %12;\
		}")
	.arg("normal")
	.arg(14)
	.arg(fonts.icon.family())
	.arg(browser.text.name(QColor::HexArgb))
	.arg(browser.text.darker(200).name(QColor::HexArgb))
	.arg(browser.background.lighter(110).name(QColor::HexRgb))
	.arg(browser.anchorActive.name(QColor::HexRgb))
	.arg(browser.anchorActive.darker(150).name(QColor::HexRgb))
	.arg(browser.background.lighter(120).name(QColor::HexRgb))
	.arg(browser.anchorActive.darker(150).name(QColor::HexRgb))
	.arg(browser.anchorActive.darker(200).name(QColor::HexArgb))
	.arg(browser.background.darker(110).name(QColor::HexRgb));
}

