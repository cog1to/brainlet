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
	QString name,
	uint64_t size
)
	: QFrame(parent),
	m_style(style),
	m_id(id),
	m_name(name),
	m_size(size),
	m_layout(this)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_label = new ElidedLabelWidget(parent, name, true);
	m_label->setStyleSheet(
		QString("background-color: #00000000; font: %1 %2px \"%3\"; color: %4;")
		.arg("bold")
		.arg(18)
		.arg(style->brains.textFont.family())
		.arg(style->brains.text.name(QColor::HexRgb))
	);

	setStyleSheet(
		getStyle(style, StatusNormal)
	);

	// Info.
	
	m_infoLabel = new QLabel(nullptr);
	m_infoLabel->setText(humanReadableSize(size));
	m_infoLabel->setStyleSheet(
		QString("background-color: #00000000; font: %1 %2px \"%3\"; color: %4;")
		.arg("normal")
		.arg(14)
		.arg(style->brains.textFont.family())
		.arg(style->brains.text.darker(150).name(QColor::HexRgb))
	);

	// Buttons.

	m_deleteButton = new QPushButton(QChar(0xf1f8), nullptr);
	m_deleteButton->setStyleSheet(
		style->brainItemButtonStyle()
	);
	m_deleteButton->setVisible(false);

	m_renameButton = new QPushButton(QChar(0xf304), nullptr);
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

	m_buttonsLayout = new QHBoxLayout(nullptr);
	m_buttonsLayout->addStretch();
	m_buttonsLayout->addWidget(m_infoLabel);
	m_buttonsLayout->addWidget(m_renameButton);
	m_buttonsLayout->addWidget(m_deleteButton);
	m_buttonsLayout->setContentsMargins(0, 0, 0, 0);

	m_label->setContentsMargins(0, 0, 0, 0);
	m_layout.setContentsMargins(0, PADDING, 0, PADDING);
	m_layout.setSpacing(SPACING);
	m_layout.addWidget(m_label);
	m_layout.setAlignment(m_label, Qt::AlignHCenter);
	m_layout.addLayout(m_buttonsLayout);
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

const uint64_t BrainItemWidget::brainSize() const {
	return m_size;
}

void BrainItemWidget::setBrainSize(uint64_t size) {
	m_size = size;

	if (m_infoLabel != nullptr) {
		m_infoLabel->setText(humanReadableSize(size));
	}
}

// Sizing

QSize BrainItemWidget::sizeHint() const {
	QSize labelSize = m_label->sizeHint();
	QSize buttonSize = m_deleteButton->sizeHint();
	return QSize(
		MAX_WIDTH,
		labelSize.height() + buttonSize.height() + PADDING * 2 + SPACING + 14
	);
}

// Events

void BrainItemWidget::enterEvent(QEnterEvent*) {
	m_deleteButton->setVisible(true);
	m_renameButton->setVisible(true);
	setStyleSheet(
		getStyle(m_style, StatusHover)
	);
	m_layout.invalidate();
}

void BrainItemWidget::leaveEvent(QEvent *) {
	m_deleteButton->setVisible(false);
	m_renameButton->setVisible(false);
	setStyleSheet(
		getStyle(m_style, StatusNormal)
	);
	m_layout.invalidate();
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
	.arg(style->brains.text.name(QColor::HexRgb))
	.arg(style->brains.text.darker(borderTint).name(QColor::HexRgb))
	.arg(style->brains.background.lighter(backTint).name(QColor::HexRgb));
}

inline QString BrainItemWidget::humanReadableSize(uint64_t size) {
	static const char* sizes[] = { "B", "KB", "MB", "GB", "TB" };

	float visibleSize = size;
	int groupIdx = 0;
	while (visibleSize >= 1024) {
		visibleSize /= 1024.0;
		groupIdx += 1;
	}

	QString formatted = QString::asprintf("%.2f%s", visibleSize, sizes[groupIdx]);
	return formatted;
}
