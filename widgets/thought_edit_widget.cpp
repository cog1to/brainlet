#include <QColor>
#include <QString>
#include <QWindow>
#include <QWidget>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QTextEdit>
#include <QFrame>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QFocusEvent>

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
		"background-color: #00000000; padding: 0; color: %1; font: %2 %3px \"%4\";"
	).arg(style->textColor().name(QColor::HexRgb))
		.arg(style->font().bold() ? "bold" : "")
		.arg(style->font().pixelSize())
		.arg(style->font().family());
	setStyleSheet(stylesheet);

	document()->setDocumentMargin(0);
	document()->setIndentWidth(0);
	setPlainText(QString::fromStdString(text));
	setAlignment(Qt::AlignCenter);
	setFocusPolicy(Qt::NoFocus);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

	installEventFilter(this);

	QObject::connect(
		this, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SIGNAL(menuRequested(const QPoint&))
	);
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
	if (event->button() == Qt::LeftButton) {
		if (isReadOnly()) {
			emit clicked();
		} else if (!isReadOnly() && !hasFocus()) {
			setFocus();
			emit editStarted();
		}
		QTextEdit::mousePressEvent(event);
	}
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
	} else if (event->key() == Qt::Key_Up) {
		emit prevSuggestion();
	} else if (event->key() == Qt::Key_Down) {
		emit nextSuggestion();
	} else if (event->key() == Qt::Key_Tab) {
		emit cycleSuggestion();
	} else if (QString text = event->text(); !text.isEmpty()) {
		QString currentText = toPlainText();
		if (
			(currentText.length() + text.length() > maxLength) &&
			(text.length() > 1 || text[0].isPrint())
		) {
			return;
		} else {
			QTextEdit::keyPressEvent(event);
		}
	} else {
		QTextEdit::keyPressEvent(event);
	}
}

void ThoughtEditWidget::focusOutEvent(QFocusEvent *event) {
	QTextEdit::focusOutEvent(event);
	emit focusLost();
}

void ThoughtEditWidget::clearFocus() {
	QTextEdit::clearFocus();
}

void ThoughtEditWidget::insertFromMimeData(const QMimeData *source) {
	if (source == nullptr)
		return;

	QString text = source->text();
	QString currentText = toPlainText();
	int availableLength = maxLength - currentText.length();

	if (availableLength  > 0) {
		qDebug() << "can paste";
		QString availableText = text.left(availableLength);
		QMimeData newData = QMimeData();
		newData.setText(availableText);
		QTextEdit::insertFromMimeData(&newData);
	}
}

