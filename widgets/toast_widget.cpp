#include <QWidget>
#include <QString>
#include <QLabel>
#include <QHBoxLayout>

#include "widgets/style.h"
#include "widgets/toast_widget.h"

ToastWidget::ToastWidget(Style *style, QString text)
	: QFrame(nullptr),
	m_layout(this), m_label(text, nullptr), m_text(text)
{
	setStyleSheet(
		QString(
			"QFrame{\
				border-radius: 8px;\
				background-color: %1;\
			}"
		)
		.arg("#dc322f")
	);

	m_label.setStyleSheet(
		QString(
			"QLabel{\
			  color: %1;\
			  font: bold %2 \"%3\";\
			  text-align: center;\
			}"
		)
		.arg("#ffffff")
		.arg(style->textEditFont().pixelSize())
		.arg(style->textEditFont().family())
	);
	m_label.setWordWrap(true);
	m_label.setAlignment(Qt::AlignCenter | Qt::AlignCenter);

	m_layout.setContentsMargins(16, 8, 16, 8);
	m_layout.addWidget(&m_label);
}

void ToastWidget::show(QWidget *parent) {
	assert(parent != nullptr);

	QSize parentSize = parent->size();
	m_label.setMaximumWidth(parentSize.width() / 3);

	QSize hint = sizeHint();
	setGeometry(
		(parentSize.width() - hint.width()) / 2,
		(parentSize.height() - hint.height() - 16),
		hint.width(),
		hint.height()
	);

	setParent(parent);
	QFrame::show();
	
	// Hide timer.
	QTimer::singleShot(5000, this, &ToastWidget::hide);
}

void ToastWidget::hide() {
	setParent(nullptr);
}

// Events.

void ToastWidget::mouseReleaseEvent(QMouseEvent*) {
	hide();
}

