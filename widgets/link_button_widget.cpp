#include <QEvent>
#include <QEnterEvent>
#include <QWidget>
#include <QPushButton>

#include "widgets/style.h"
#include "widgets/link_button_widget.h"

LinkButtonWidget::LinkButtonWidget(QWidget *parent, Style *style)
	: QPushButton(parent), m_style(style) 
{
	setStyleSheet(
		QString("background-color: #00000000;\
			color: %1;\
			font: bold %2px \"%3\";\
			border-width: 1px;\
			border-color: %4;\
			border-radius: 8px;\
			border-style: solid;\
			padding: 2px; padding-left: 8px; padding-right: 8px;"
		)
		.arg(m_style->browser.text.name(QColor::HexRgb))
		.arg(m_style->browser.browseFont.pixelSize() - 2)
		.arg(m_style->browser.browseFont.family())
		.arg(m_style->browser.text.darker().name(QColor::HexRgb))
	);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setFocusPolicy(Qt::NoFocus);
}

void LinkButtonWidget::enterEvent(QEnterEvent *) {
	setStyleSheet(
		QString("background-color: %1;\
			color: %2;\
			font: bold %3px \"%4\";\
			border-width: 1px;\
			border-color: %5;\
			border-radius: 8px;\
			border-style: solid;\
			padding: 2px; padding-left: 8px; padding-right: 8px;"
		)
		.arg(m_style->browser.text.name(QColor::HexRgb))
		.arg(m_style->browser.background.name(QColor::HexRgb))
		.arg(m_style->browser.browseFont.pixelSize() - 2)
		.arg(m_style->browser.browseFont.family())
		.arg(m_style->browser.text.name(QColor::HexRgb))
	);
}

void LinkButtonWidget::leaveEvent(QEvent *) {
	setStyleSheet(
		QString("background-color: #00000000;\
			color: %1;\
			font: bold %2px \"%3\";\
			border-width: 1px;\
			border-color: %4;\
			border-radius: 8px;\
			border-style: solid;\
			padding: 3px; padding-left: 8px; padding-right: 8px;"
		)
		.arg(m_style->browser.text.name(QColor::HexRgb))
		.arg(m_style->browser.browseFont.pixelSize() - 2)
		.arg(m_style->browser.browseFont.family())
		.arg(m_style->browser.text.darker().name(QColor::HexRgb))
	);
}

void LinkButtonWidget::mousePressEvent(QMouseEvent *event) {
	setStyleSheet(
		QString("background-color: %1;\
			color: %2;\
			font: bold %3px \"%4\";\
			border-width: 2px;\
			border-color: %5;\
			border-radius: 8px;\
			border-style: solid;\
			padding: 0px; padding-left: 7px; padding-right: 7px;"
		)
		.arg(m_style->browser.text.darker(105).name(QColor::HexRgb))
		.arg(m_style->browser.background.darker().name(QColor::HexRgb))
		.arg(m_style->browser.browseFont.pixelSize() - 2)
		.arg(m_style->browser.browseFont.family())
		.arg(m_style->browser.background.lighter().name(QColor::HexRgb))
	);

	QPushButton::mousePressEvent(event);
}

void LinkButtonWidget::mouseReleaseEvent(QMouseEvent *event) {
	setStyleSheet(
		QString("background-color: %1;\
			color: %2;\
			font: bold %3px \"%4\";\
			border-width: 1px;\
			border-color: %5;\
			border-radius: 8px;\
			border-style: solid;\
			padding: 2px; padding-left: 8px; padding-right: 8px;"
		)
		.arg(m_style->browser.text.name(QColor::HexRgb))
		.arg(m_style->browser.background.name(QColor::HexRgb))
		.arg(m_style->browser.browseFont.pixelSize() - 2)
		.arg(m_style->browser.browseFont.family())
		.arg(m_style->browser.text.name(QColor::HexRgb))
	);

	QPushButton::mouseReleaseEvent(event);
}

