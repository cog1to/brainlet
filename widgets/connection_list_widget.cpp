#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

#include "widgets/base_widget.h"
#include "widgets/connection_list_widget.h"
#include "widgets/link_button_widget.h"

// Item.

ConnectionItemWidget::ConnectionItemWidget(
	QWidget *parent,
	Style *style,
	bool showButtons,
	ThoughtId id,
	QString name
) : BaseWidget(parent, style),
	m_id(id),
	m_name(name),
	m_layout(this),
	m_showButtons(showButtons)
{
	setMinimumWidth(300);
	m_layout.setContentsMargins(QMargins(5, 4, 5, 4));
	setStyleSheet(QString("background: #00000000"));
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

	// Setup title.
	QLabel *titleLabel = new QLabel(nullptr);
	titleLabel->setStyleSheet(
		QString("color: %1; font: %2px \"%3\"")
			.arg(style->textColor().name(QColor::HexRgb))
			.arg(style->font().pixelSize())
			.arg(style->font().family())
	);
	titleLabel->setText(name);
	titleLabel->setMinimumWidth(40);
	titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_layout.addWidget(titleLabel);

	// Setup buttons.
	if (showButtons) {
		m_linkButton = makeButton(style, tr("← Link"));
		connect(m_linkButton, SIGNAL(clicked()), this, SLOT(onLinkClicked()));

		m_childButton = makeButton(style, tr("↓ Down"));
		connect(m_childButton, SIGNAL(clicked()), this, SLOT(onChildClicked()));

		m_parentButton = makeButton(style, tr("↑ Up"));
		connect(m_parentButton, SIGNAL(clicked()), this, SLOT(onParentClicked()));

		m_layout.addWidget(m_linkButton);
		m_layout.addWidget(m_childButton);
		m_layout.addWidget(m_parentButton);
	}
}

QPushButton *ConnectionItemWidget::makeButton(Style *style, QString title) {
	LinkButtonWidget *button = new LinkButtonWidget(nullptr, style);

	button->setText(title);
	button->setVisible(false);

	return button;
}

// Events.

void ConnectionItemWidget::paintEvent(QPaintEvent* event) {
	QPainter painter(this);

	if (m_pressed) {
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(255, 255, 255, 32));
		painter.drawRect(rect());
	} else if (m_hover) {
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(255, 255, 255, 16));
		painter.drawRect(rect());
	}

	QWidget::paintEvent(event);
}

void ConnectionItemWidget::mousePressEvent(QMouseEvent *) {
	if (m_showButtons)
		return;

	m_pressed = true;
	update();
}

void ConnectionItemWidget::mouseReleaseEvent(QMouseEvent *event) {
	if (!m_pressed)
		return;

	m_pressed = false;
	update();

	QPoint pos = event->pos();
	QSize current = size();
	if (
		pos.x() >= 0 && pos.x() <= current.width() &&
		pos.y() >= 0 && pos.y() <= current.height()
	) {
		emit clicked(this, m_id, m_name);
	}
}

void ConnectionItemWidget::enterEvent(QEnterEvent *) {
	m_hover = true;

	if (m_showButtons) {
		m_linkButton->show();
		m_parentButton->show();
		m_childButton->show();
	}

	update();
}

void ConnectionItemWidget::leaveEvent(QEvent *) {
	m_hover = false;

	if (m_showButtons) {
		m_linkButton->hide();
		m_parentButton->hide();
		m_childButton->hide();
	}

	update();
}

// Slots

void ConnectionItemWidget::onLinkClicked() {
	emit buttonClicked(this, m_id, ConnButtonLink);
}

void ConnectionItemWidget::onChildClicked() {
	emit buttonClicked(this, m_id, ConnButtonChild);
}

void ConnectionItemWidget::onParentClicked() {
	emit buttonClicked(this, m_id, ConnButtonParent);
}

// List.

ConnectionListWidget::ConnectionListWidget(
	QWidget *parent,
	Style *style,
	bool showButtons
) : BaseWidget(parent, style),
	m_layout(this),
	m_showButtons(showButtons)
{
	m_layout.setSpacing(0);
	m_layout.setContentsMargins(QMargins(0, 0, 0, 0));
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

const std::vector<ConnectionItem> &ConnectionListWidget::items() {
	return m_items;
}

void ConnectionListWidget::setItems(std::vector<ConnectionItem> items) {
	m_items = items;

	// Delete old items.
	QLayoutItem *item = nullptr;
	while ((item = m_layout.takeAt(0)) != nullptr) {
		delete item->widget();
		delete item;
	}

	// Add new items.
	for (int idx = 0; idx < std::min((int)items.size(), MaxItems); idx++) {
		auto item = items[idx];

		ConnectionItemWidget *widget = new ConnectionItemWidget(
			nullptr, m_style, m_showButtons,
			item.id, item.name
		);

		connect(
			widget,
			SIGNAL(buttonClicked(ConnectionItemWidget*, ThoughtId, ConnectionItemButton)),
			this,
			SLOT(onConnectionSelected(ConnectionItemWidget*, ThoughtId, ConnectionItemButton))
		);

		connect(
			widget, SIGNAL(clicked(ConnectionItemWidget*, ThoughtId, QString)),
			this, SLOT(onThoughtSelected(ConnectionItemWidget*, ThoughtId, QString))
		);

		m_layout.addWidget(widget);

		if (idx != items.size() - 1) {
			m_layout.addWidget(makeSeparator());
		}
	}

	m_layout.invalidate();
	adjustSize();
}

QWidget *ConnectionListWidget::makeSeparator() {
	QWidget *separator = new QWidget(nullptr);

	separator->setMinimumSize(1, 1);
	separator->setMaximumHeight(1);
	separator->setStyleSheet(
		QString("background-color: %1")
			.arg(m_style->textEditColor().name(QColor::HexRgb))
	);
	separator->setSizePolicy(
		QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum)
	);

	return separator;
}

QSize ConnectionListWidget::sizeHint() const {
	int maxWidth = 0;
	int maxHeight = 0;

	QFontMetrics metrics(m_style->font());
	for (auto idx = 0; idx < m_items.size(); idx++) {
		QRect bounds = metrics.boundingRect(m_items[idx].name);
		QSize textSize = bounds.size();

		maxHeight = maxHeight + metrics.height() + 8;
		maxWidth = std::max(maxWidth, textSize.width());

		if (idx < m_items.size() - 1) {
			// Spacing for separator.
			maxHeight += (1 + m_layout.spacing() * 2);
		}
	}

	// Add margins.
	QMargins margins = m_layout.contentsMargins();
	maxWidth += margins.left() + margins.right();
	maxHeight += margins.top() + margins.bottom();

	return QSize(maxWidth, maxHeight);
}

// Slots.

void ConnectionListWidget::onConnectionSelected(
	ConnectionItemWidget*,
	ThoughtId id,
	ConnectionItemButton button
) {
	switch (button) {
		case ConnButtonLink:
			emit connectionSelected(id, ConnectionType::link, false);
			break;
		case ConnButtonParent:
			emit connectionSelected(id, ConnectionType::child, true);
			break;
		case ConnButtonChild:
			emit connectionSelected(id, ConnectionType::child, false);
			break;
	}
}

void ConnectionListWidget::onThoughtSelected(
	ConnectionItemWidget*,
	ThoughtId id,
	QString name
) {
	emit thoughtSelected(id, name);
}

