#include <QWidget>
#include <QFrame>
#include <QTextEdit>
#include <QVBoxLayout>

#include "model/thought.h"
#include "widgets/search_widget.h"
#include "widgets/thought_edit_widget.h"

SearchWidget::SearchWidget(
	QWidget *parent,
	Style *style,
	bool showButtons
) : QFrame(parent),
	m_style(style),
	m_showButtons(showButtons),
	m_layout(this),
	m_list(nullptr, style, showButtons),
	m_separator(nullptr)
{
	// Setup separator.
	m_separator.setMinimumSize(1, 1);
	m_separator.setStyleSheet(
		QString("background-color: %1; border-radius: 10px;")
			.arg(m_style->textEditColor().name(QColor::HexRgb))
	);
	m_separator.setSizePolicy(
		QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum)
	);
	m_separator.setVisible(false);
	m_list.setVisible(false);

	// Edit styling.
	m_edit = new ThoughtEditWidget(nullptr, style, false, "");
	QFontMetrics metrics(m_style->font());
	m_edit->setMaximumHeight(metrics.height() + 2);
	m_edit->setAlignment(Qt::AlignLeft);
	m_edit->setWordWrapMode(QTextOption::NoWrap);
	m_edit->setPlaceholderText("Go to...");

	// Setup icon.
	m_icon = new QLabel(nullptr);

	QColor iconColor = style->textColor();
	iconColor.setAlpha(128);

	m_icon->setStyleSheet(
		QString("background: #00000000; font: %1px \"%2\"; color: %3")
			.arg(style->iconFont().pixelSize())
			.arg(style->iconFont().family())
			.arg(iconColor.name(QColor::HexArgb))
	);
	m_icon->setText(QChar(0xf002));

	// Setup input layout.
	m_input_layout = new QHBoxLayout(nullptr);
	m_input_layout->setContentsMargins(QMargins(6, 0, 6, 0));
	m_input_layout->setSpacing(6);
	m_input_layout->addWidget(m_icon);
	m_input_layout->addWidget(m_edit);

	// Setup container layout.
	m_layout.setContentsMargins(QMargins(5, 5, 5, 5));
	m_layout.setSpacing(0);
	m_layout.addLayout(m_input_layout);
	m_layout.addWidget(&m_separator);
	m_layout.addWidget(&m_list);

	// Style.
	setStyleSheet(
		QString("background: #55000000; border-radius: 10px;")
	);

	// Events.
	connect(
		m_edit, SIGNAL(editStarted()),
		this, SLOT(onTextEdit())
	);

	connect(
		m_edit, SIGNAL(editCanceled()),
		this, SLOT(onTextCancel())
	);

	connect(
		m_edit, SIGNAL(textChanged()),
		this, SLOT(onTextChanged())
	);

	connect(
		&m_list, SIGNAL(thoughtSelected(ThoughtId)),
		this, SLOT(onThoughtSelected(ThoughtId))
	);

	connect(
		&m_list, SIGNAL(connectionSelected(ThoughtId, ConnectionType, bool)),
		this, SLOT(onConnectionSelected(ThoughtId, ConnectionType, bool))
	);
}

void SearchWidget::setItems(std::vector<ConnectionItem> items) {
	if (items.size() == 0) {
		m_separator.setVisible(false);
		m_list.setVisible(false);
	} else {
		m_list.setItems(items);

		m_separator.setVisible(true);
		m_list.setVisible(true);
	}
}

void SearchWidget::clear() {
	std::vector<ConnectionItem> emptyList;
	setItems(emptyList);

	QString emptyString = "";
	m_edit->setPlainText(emptyString);
	m_edit->clearFocus();
}

// State.

bool SearchWidget::isActive() const {
	return m_edit->hasFocus();
}

// Sizing.

QSize SearchWidget::sizeHint() const {
	return m_layout.sizeHint();
}

// Slots.

// Text editing.

void SearchWidget::onTextChanged() {
	QString text = m_edit->toPlainText();
	restyleIcon(text.isEmpty() == false);

	emit textChanged(this, text);
}

void SearchWidget::onTextEdit() {
	emit searchActivated(this);
}

void SearchWidget::onTextCancel() {
	emit searchCanceled(this);
}

// List selection.

void SearchWidget::onThoughtSelected(ThoughtId id) {
	emit thoughtSelected(this, id);
}

void SearchWidget::onConnectionSelected(
	ThoughtId id,
	ConnectionType type,
	bool inc
) {
	emit connectionSelected(this, id, type, inc);
}

// Helpers

void SearchWidget::restyleIcon(bool hasText) {
	if (m_style == nullptr)
		return;

	QColor iconColor = m_style->textColor();
	if (!hasText)
		iconColor.setAlpha(128);

	m_icon->setStyleSheet(
		QString("background: #00000000; font: %1px \"%2\"; color: %3")
			.arg(m_style->iconFont().pixelSize())
			.arg(m_style->iconFont().family())
			.arg(iconColor.name(QColor::HexArgb))
	);
}
