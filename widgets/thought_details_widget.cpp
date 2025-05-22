#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "widgets/thought_details_widget.h"
#include "widgets/markdown_scroll_widget.h"

ThoughtDetailsWidget::ThoughtDetailsWidget(
	QWidget *parent,
	Style *style,
	MarkdownScrollWidget *markdown
) : BaseWidget(parent, style)
{
	m_markdown = markdown;
	m_layout = new QVBoxLayout(this);
	m_title = new QLabel(nullptr);
	m_separator = new QWidget(nullptr);

	setStyleSheet(
		QString("background-color: %1").arg(
			style->background().name(QColor::HexRgb)
		)
	);

	m_title->setStyleSheet(
		QString("color: %1; font: bold %2px \"%3\"")
			.arg(style->textEditColor().name(QColor::HexRgb))
			.arg(style->textEditFont().pixelSize() * 2.0)
			.arg(style->textEditFont().family())
	);
	m_title->setWordWrap(true);
	m_title->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

	m_separator->setMinimumSize(1, 1);
	m_separator->setStyleSheet(
		QString("background-color: %1")
			.arg(style->textEditColor().name(QColor::HexRgb))
	);
	m_separator->setSizePolicy(
		QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum)
	);

	m_layout->setSpacing(10);
	m_layout->setContentsMargins(QMargins(5, 5, 5, 5));
	m_layout->addWidget(m_title);
	m_layout->addWidget(m_separator);
	m_layout->addWidget(markdown);
}

void ThoughtDetailsWidget::setTitle(QString title) {
	m_title->setText(title);
}

