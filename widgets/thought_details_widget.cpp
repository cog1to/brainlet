#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "widgets/thought_details_widget.h"
#include "widgets/markdown_scroll_widget.h"
#include "widgets/wrapping_text_edit_widget.h"

ThoughtDetailsWidget::ThoughtDetailsWidget(
	QWidget *parent,
	Style *style,
	MarkdownScrollWidget *markdown
) : BaseWidget(parent, style)
{
	m_markdown = markdown;
	m_layout = new QVBoxLayout(this);
	m_title = new WrappingTextEditWidget(nullptr);
	m_separator = new QWidget(nullptr);

	setStyleSheet(
		QString("background-color: %1").arg(
			style->editor.background.name(QColor::HexRgb)
		)
	);

	m_title->setStyleSheet(
		QString("color: %1; font: bold %2px \"%3\"")
			.arg(style->editor.text.name(QColor::HexRgb))
			.arg(style->editor.textFont.pixelSize() * 2.0)
			.arg(style->editor.textFont.family())
	);
	m_title->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

	m_separator->setMinimumSize(1, 1);
	m_separator->setStyleSheet(
		QString("background-color: %1")
			.arg(style->editor.text.name(QColor::HexRgb))
	);
	m_separator->setSizePolicy(
		QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum)
	);

	m_layout->setSpacing(10);
	m_layout->setContentsMargins(QMargins(5, 5, 5, 5));
	m_layout->addWidget(m_title);
	m_layout->addWidget(m_separator);
	m_layout->addWidget(markdown);

	QObject::connect(
		m_title, SIGNAL(editConfirmed(QString&, std::function<void(bool)>)),
		this, SLOT(onTitleEditConfirmed(QString&, std::function<void(bool)>))
	);
}

ThoughtDetailsWidget::~ThoughtDetailsWidget() {
	delete m_layout;
}

void ThoughtDetailsWidget::onTitleEditConfirmed(QString& newTitle, std::function<void(bool)> callback) {
	emit titleEditConfirmed(newTitle, callback);
}

void ThoughtDetailsWidget::setTitle(QString title) {
	m_title->setTitle(title);
	m_title->updateGeometry();
	m_layout->update();
}

