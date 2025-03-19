#include <QSizePolicy>
#include <QWidget>

#include "widgets/brain_widget.h"

BrainWidget::BrainWidget(
	QWidget *parent,
	Style *style,
	ContainerWidget *container,
	ThoughtDetailsWidget *details
)
	: BaseWidget(parent, style),
	m_layout(this), m_canvasContainer(container), m_details(details)
{
	setStyleSheet(
		QString("background-color: %1").arg(
			style->background().name(QColor::HexRgb)
		)
	);

	QWidget *separator = new QWidget(nullptr);
	separator->setMinimumSize(1, 1);
	separator->setStyleSheet(
		QString("background-color: %1")
			.arg(style->textEditColor().name(QColor::HexRgb))
	);
	separator->setSizePolicy(
		QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding)
	);

	m_layout.addWidget(container, 1);
	m_layout.addWidget(separator, 0);
	m_layout.addWidget(details, 1);
}

ThoughtDetailsWidget *BrainWidget::details() {
	return m_details;
}

ContainerWidget *BrainWidget::canvas() {
	return m_canvasContainer;
}
