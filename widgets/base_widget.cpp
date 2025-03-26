#include <QWidget>

#include "widgets/base_widget.h"

BaseWidget::BaseWidget(QWidget *parent, Style *style): QWidget(parent) {
	m_style = style;
}

Style *BaseWidget::style() {
	return m_style;
}
