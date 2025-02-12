#include <QFont>
#include <QColor>
#include <QObject>

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
	QColor anchorHighlight
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
}

Style& Style::defaultStyle() {
	static QFont font = QFont("Noto Sans");
	font.setPixelSize(12);

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
		QColor(255, 255, 255, 255)
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
