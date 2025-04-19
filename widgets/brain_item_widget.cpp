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
	: QFrame(parent),
	m_style(style),
	m_id(id),
	m_name(name),
	m_layout(this)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_label = new ElidedLabelWidget(parent, name);
	m_label->setStyleSheet(
		QString("background-color: #00000000; font: %1 %2px \"%3\"; color: %4;")
		.arg("bold")
		.arg(18)
		.arg(style->font().family())
		.arg(style->textColor().name(QColor::HexRgb))
	);

	setStyleSheet(
		getStyle(style, StatusNormal)
	);

	m_deleteButton = new QPushButton(tr("Delete"), nullptr);
	m_deleteButton->setStyleSheet(
		style->brainItemButtonStyle()
	);
	m_deleteButton->setVisible(false);

	m_renameButton = new QPushButton(tr("Rename"), nullptr);
	m_renameButton->setStyleSheet(
		style->brainItemButtonStyle()
	);
	m_renameButton->setVisible(false);

	connect(
		m_deleteButton, &QPushButton::clicked,
		this, &BrainItemWidget::onDeleteClick
	);

	connect(
		m_renameButton, &QPushButton::clicked,
		this, &BrainItemWidget::onRenameClick
	);

	m_label->setContentsMargins(0, 1, 0, 1);
	m_layout.setContentsMargins(0, 0, 0, 0);
	m_layout.addWidget(m_label);
	m_layout.addWidget(m_renameButton);
	m_layout.addWidget(m_deleteButton);
}

const QString BrainItemWidget::id() const {
	return m_id;
}

const QString BrainItemWidget::name() const {
	return m_name;
}

void BrainItemWidget::setName(QString name) {
	m_name = name;

	if (m_label != nullptr)
		m_label->setText(name);
}

// Sizing

QSize BrainItemWidget::sizeHint() const {
	QSize labelSize = m_label->sizeHint();
	return QSize(
		labelSize.width(),
		labelSize.height() + 18
	);	
}

// Events

void BrainItemWidget::enterEvent(QEnterEvent*) {
	m_deleteButton->setVisible(true);
	m_renameButton->setVisible(true);
	setStyleSheet(
		getStyle(m_style, StatusHover)
	);
}

void BrainItemWidget::leaveEvent(QEvent *) {
	m_deleteButton->setVisible(false);
	m_renameButton->setVisible(false);
	setStyleSheet(
		getStyle(m_style, StatusNormal)
	);
}

void BrainItemWidget::mousePressEvent(QMouseEvent*) {
	setStyleSheet(
		getStyle(m_style, StatusPressed)
	);
}

void BrainItemWidget::mouseReleaseEvent(QMouseEvent *event) {
	QPoint pos = event->pos();
	QSize s = size();

	if (
		pos.x() >= 0 && pos.y() >= 0 &&
		pos.x() <= s.width() && pos.y() < s.height()
	) {
		emit buttonClicked(this);
		setStyleSheet(getStyle(m_style, StatusHover));
	} else {
		setStyleSheet(getStyle(m_style, StatusNormal));
	}
}

// Slots

void BrainItemWidget::onClick() {
	emit buttonClicked(this);
}

void BrainItemWidget::onDeleteClick() {
	emit deleteClicked(this);
}

void BrainItemWidget::onRenameClick() {
	emit renameClicked(this);
}

// Helpers

inline QString BrainItemWidget::getStyle(Style *style, Status status) {
	int backTint = 110;
	int borderTint = 120;

	switch (status) {
		case StatusNormal:
			break;
		case StatusHover:
			backTint = 130;
			break;
		case StatusPressed:
			backTint = 90;
			borderTint = 300;
			break;
	}

	return QString("BrainItemWidget{\
		color: %1;\
		border-width: 1px;\
		border-color: %2;\
		padding: 0px;\
		padding-left: 12px;\
		padding-right: 12px;\
		background-color: %3;\
		border-radius: 10px;\
		border-style: solid;\
	}")
	.arg(style->textEditColor().name(QColor::HexRgb))
	.arg(style->textEditColor().darker(borderTint).name(QColor::HexRgb))
	.arg(style->background().lighter(backTint).name(QColor::HexRgb));
}

