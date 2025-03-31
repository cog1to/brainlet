#include <QPushButton>
#include <QColor>
#include <QMargins>
#include <QEvent>
#include <QResizeEvent>
#include <QEnterEvent>

#include "widgets/brain_item_widget.h"

BrainItemWidget::BrainItemWidget(
	QWidget *parent,
	Style *style,
	QString id,
	QString name
)
	: QPushButton(name, parent),
	m_style(style),
	m_id(id),
	m_name(name),
	m_deleteButton(tr("Delete"), this)
{
	setStyleSheet(
		QString("QPushButton{\
				font: %1 %2px \"%3\";\
				color: %4;\
				border-width: 1px;\
				border-color: %5;\
				padding: 8px;\
				padding-left: 12px;\
				padding-right: 12px;\
				background-color: %6;\
				text-align: left;\
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
		.arg(style->font().family())
		.arg(style->textColor().name(QColor::HexArgb))
		.arg(style->textColor().darker(200).name(QColor::HexArgb))
		.arg(style->background().lighter(110).name(QColor::HexRgb))
		.arg(style->textColor().darker(300).name(QColor::HexArgb))
		.arg(style->background().darker(110).name(QColor::HexRgb))
		.arg(style->background().lighter(130).name(QColor::HexRgb))
	);

	m_deleteButton.setStyleSheet(
		QString("QPushButton{\
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
				background-color: %9;\
			}\
			QPushButton:hover:pressed{\
				border-color: %7;\
				background-color: %8;\
			}")
		.arg("normal")
		.arg(14)
		.arg(style->font().family())
		.arg(style->textColor().name(QColor::HexArgb))
		.arg(style->textColor().darker(200).name(QColor::HexArgb))
		.arg(style->background().lighter(110).name(QColor::HexRgb))
		.arg(style->textColor().darker(300).name(QColor::HexArgb))
		.arg(style->background().darker(110).name(QColor::HexRgb))
		.arg(style->background().lighter(120).name(QColor::HexRgb))
	);

	m_deleteButton.setVisible(false);

	connect(
		&m_deleteButton, &QPushButton::clicked,
		this, &BrainItemWidget::onDeleteClick
	);
	connect (
		this, &QPushButton::clicked,
		this, &BrainItemWidget::onClick
	);
}

const QString BrainItemWidget::id() const {
	return m_id;
}

const QString BrainItemWidget::name() const {
	return m_name;
}

// Events

void BrainItemWidget::resizeEvent(QResizeEvent *event) {
	QSize current = event->size();

	QSize hint = m_deleteButton.sizeHint();
	m_deleteButton.setGeometry(
		current.width() - 12 - hint.width(),
		(current.height() - hint.height()) / 2,
		hint.width(), hint.height()
	);
}

void BrainItemWidget::enterEvent(QEnterEvent*) {
	m_deleteButton.setVisible(true);
}

void BrainItemWidget::leaveEvent(QEvent *) {
	m_deleteButton.setVisible(false);
}

// Slots

void BrainItemWidget::onClick() {
	emit buttonClicked(this);
}

void BrainItemWidget::onDeleteClick() {
	emit deleteClicked(this);
}
