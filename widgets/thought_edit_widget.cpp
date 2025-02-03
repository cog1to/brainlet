#include <QColor>
#include <QGuiApplication>

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
	if (readOnly) {
		setTextInteractionFlags(Qt::NoTextInteraction);
		setCursor(Qt::ArrowCursor);
	}

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
	installEventFilter(this);
}

void ThoughtEditWidget::enterEvent(QEnterEvent*) {
	// TODO: Is there a better way to do this?
	QGuiApplication::setOverrideCursor(isReadOnly() ? Qt::ArrowCursor : Qt::IBeamCursor);
	emit mouseEnter();
}

void ThoughtEditWidget::leaveEvent(QEvent*) {
	// TODO: Is there a better way to do this?
	QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
	emit mouseLeave();
}

bool ThoughtEditWidget::eventFilter(QObject *obj, QEvent *event) {
	if (
		event->type() == QEvent::KeyPress ||
		event->type() == QEvent::Enter ||
		event->type() == QEvent::Leave ||
		event->type() == QEvent::MouseButtonPress
	) {
		return QObject::eventFilter(obj, event);
	}

	return false;
}

// Focus and keyboard.

void ThoughtEditWidget::mousePressEvent(QMouseEvent* event) {
	if (isReadOnly()) {
		emit clicked();
	} else if (!isReadOnly() && !hasFocus()) {
		setFocus();
		emit editStarted();
	}
	QTextEdit::mousePressEvent(event);
}

void ThoughtEditWidget::keyPressEvent(QKeyEvent *event) {
	if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
		emit editConfirmed([this](bool result){
			if (result) {
				this->clearFocus();
			}
		});
	} else if (event->key() == Qt::Key_Escape) {
		emit editCanceled();
		clearFocus();
	} else {
		QTextEdit::keyPressEvent(event);
	}
}

void ThoughtEditWidget::clearFocus() {
	QWidget::clearFocus();
	emit focusLost();
}
