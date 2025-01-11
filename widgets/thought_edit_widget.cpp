#include <QColor>

#include "widgets/thought_edit_widget.h"
#include "widgets/style.h"

ThoughtEditWidget::ThoughtEditWidget(
	QWidget *parent,
	Style *style,
	bool readOnly,
	std::string text
)
	: QTextEdit(parent)
{
	setReadOnly(readOnly);
	setFrameStyle(QFrame::NoFrame);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAcceptRichText(false);

	QString stylesheet = QString(
		"background-color: #00000000; padding: 0; color: %1; font: %2 %3pt \"%4\";"
	).arg(style->textColor().name(QColor::HexRgb))
		.arg(style->font().bold() ? "bold" : "")
		.arg(style->font().pointSize())
		.arg(style->font().family());
	setStyleSheet(stylesheet);

	document()->setDocumentMargin(0);
	document()->setIndentWidth(0);
	setPlainText(QString::fromStdString(text));
	setAlignment(Qt::AlignCenter);
	setFocusPolicy(Qt::NoFocus);
}

void ThoughtEditWidget::enterEvent(QEnterEvent*) {
	emit mouseEnter();
}

void ThoughtEditWidget::leaveEvent(QEvent*) {
	emit mouseLeave();
}

// Focus and keyboard.

void ThoughtEditWidget::mousePressEvent(QMouseEvent* event) {
	if (!hasFocus()) {
		setFocus();
	}
	QTextEdit::mousePressEvent(event);
}

void ThoughtEditWidget::keyPressEvent(QKeyEvent *event) {
	if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
		clearFocus();
	} else {
		QTextEdit::keyPressEvent(event);
	}
}

void ThoughtEditWidget::clearFocus() {
	QWidget::clearFocus();
	emit focusLost();
}
