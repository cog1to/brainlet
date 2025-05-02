#include <QFont>
#include <QColor>
#include <QObject>
#include <QFontDatabase>

#include "widgets/style.h"

Style::Style(
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
) {
	m_background = background;
	m_nodeBackground = nodeBackground;
	m_borderWidth = borderWidth;
	m_borderColor = borderColor;
	m_hoverBorderWidth = hoverBorderWidth;
	m_hoverBorderColor = hoverBorderColor;
	m_font = font;
	m_activeAnchorColor = activeAnchorColor;
	m_textColor = textColor;
	m_hoverBackground = hoverBackground;
	m_scrollWidth = scrollWidth;
	m_anchorHighlight = anchorHighlight;
	m_textEditFont = textEditFont;
	m_textEditColor = textEditColor;
	m_codeFont = codeFont;
	m_codeBackground = codeBackground;
	m_linkColor = linkColor;
	m_iconFont = iconFont;
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

	static Style style(
		QColor(23, 43, 52, 255),
		QColor(16, 31, 38, 128),
		1.0,
		QColor(248, 144, 87, 255),
		2.0,
		QColor(248, 144, 87, 255),
		font,
		QColor(228, 83, 75, 255),
		QColor(215, 221, 232, 255),
		QColor(0, 0, 0, 192),
		12,
		QColor(255, 255, 255, 255),
		textFont,
		QColor(185, 195, 195, 255),
		codeFont,
		QColor(43, 63, 72, 255),
		QColor(162, 187, 219),
		iconFont
	);

	return style;
}

const QColor Style::background() const {
	return m_background;
}

void Style::setBackground(QColor color) {
	m_background = color;
	emit styleChanged(this);
}

const QColor Style::nodeBackground() const {
	return m_nodeBackground;
}

void Style::setNodeBackground(QColor color) {
	m_nodeBackground = color;
	emit styleChanged(this);
}

const float Style::borderWidth() const {
	return m_borderWidth;
}

void Style::setBorderWidth(float width) {
	m_borderWidth = width;
	emit styleChanged(this);
}

const QColor Style::borderColor() const {
	return m_borderColor;
}

void Style::setBorderColor(QColor color) {
	m_borderColor = color;
	emit styleChanged(this);
}

const float Style::hoverBorderWidth() const {
	return m_hoverBorderWidth;
}

void Style::setHoverBorderWidth(float width) {
	m_hoverBorderWidth = width;
	emit styleChanged(this);
}

const QColor Style::hoverBorderColor() const {
	return m_hoverBorderColor;
}

void Style::setHoverBorderColor(QColor color) {
	m_hoverBorderColor = color;
	emit styleChanged(this);
}

const QFont Style::font() const {
	return m_font;
}

void Style::setFont(QFont font) {
	m_font = font;
	emit styleChanged(this);
}

const QColor Style::activeAnchorColor() const {
	return m_activeAnchorColor;
}

void Style::setActiveAnchorColor(QColor color) {
	m_activeAnchorColor = color;
	emit styleChanged(this);
}

const QColor Style::textColor() const {
	return m_textColor;
}

void Style::setTextColor(QColor color) {
	m_textColor = color;
	emit styleChanged(this);
}

const QColor Style::hoverBackground() const {
	return m_hoverBackground;
}

void Style::setHoverBackground(QColor color) {
	m_hoverBackground = color;
	emit styleChanged(this);
}

const int Style::scrollWidth() const {
	return m_scrollWidth;
}

void Style::setScrollWidth(int value) {
	m_scrollWidth = value;
	emit styleChanged(this);
}

const QColor Style::anchorHighlight() const {
	return m_anchorHighlight;
}

void Style::setAnchorHighlight(QColor value) {
	m_anchorHighlight = value;
	emit styleChanged(this);
}

const QFont Style::textEditFont() const {
	return m_textEditFont;
}

void Style::setTextEditFont(QFont font) {
	m_textEditFont = font;
	emit styleChanged(this);
}

const QColor Style::textEditColor() const {
	return m_textEditColor;
}

void Style::setTextEditColor(QColor value) {
	m_textEditColor = value;
	emit styleChanged(this);
}

const QFont Style::codeFont() const {
	return m_codeFont;
}

void Style::setCodeFont(QFont font) {
	m_codeFont = font;
	emit styleChanged(this);
}

const QColor Style::codeBackground() const {
	return m_codeBackground;
}

void Style::setCodeBackground(QColor color) {
	m_codeBackground = color;
	emit styleChanged(this);
}

const QColor Style::linkColor() const {
	return m_linkColor;
}

void Style::setLinkColor(QColor color) {
	m_linkColor = color;
	emit styleChanged(this);
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
	).arg(m_textColor.name(QColor::HexRgb))
	.arg(m_font.bold() ? "bold" : "")
	.arg(m_font.pixelSize())
	.arg(m_font.family());

	return stylesheet;
}

QString Style::brainListButtonStyle(QString align, QColor foreground) {
	QString stylesheet = QString(
		"QPushButton{\
			font: %1 %2px \"%3\";\
			color: %4;\
			border-width: 1px;\
			border-color: %5;\
			padding: 8px;\
			padding-left: 12px;\
			padding-right: 12px;\
			background-color: %6;\
			text-align: %10;\
			border-radius: 10px;\
			border-style: solid;\
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
		.arg(m_font.family())
		.arg(foreground.name(QColor::HexArgb))
		.arg(foreground.darker(200).name(QColor::HexArgb))
		.arg(m_background.lighter(110).name(QColor::HexRgb))
		.arg(foreground.darker(300).name(QColor::HexArgb))
		.arg(m_background.darker(110).name(QColor::HexRgb))
		.arg(m_background.lighter(130).name(QColor::HexRgb))
		.arg(align);

	return stylesheet;
}

QString Style::brainItemButtonStyle() {
	return QString("QPushButton{\
			font: %1 %2px \"%3\";\
			color: %4;\
			border-width: 1px;\
			border-color: %5;\
			padding: 5px;\
			padding-left: 12px;\
			padding-right: 12px;\
			background-color: %6;\
			text-align: left;\
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
	.arg(font().family())
	.arg(textColor().name(QColor::HexArgb))
	.arg(textColor().darker(200).name(QColor::HexArgb))
	.arg(background().lighter(110).name(QColor::HexRgb))
	.arg(activeAnchorColor().name(QColor::HexRgb))
	.arg(activeAnchorColor().darker(150).name(QColor::HexRgb))
	.arg(background().lighter(120).name(QColor::HexRgb))
	.arg(activeAnchorColor().darker(150).name(QColor::HexRgb))
	.arg(activeAnchorColor().darker(200).name(QColor::HexArgb))
	.arg(background().darker(110).name(QColor::HexRgb));
}

const QFont Style::iconFont() const {
	return m_iconFont;
}

void Style::setIconFont(QFont font) {
	m_iconFont = font;
	emit styleChanged(this);
}

