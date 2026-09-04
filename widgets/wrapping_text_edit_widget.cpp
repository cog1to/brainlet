#include <QWidget>
#include <QTextEdit>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QString>
#include <QFont>
#include <QFontMetrics>
#include <QMimeData>

#include "widgets/wrapping_text_edit_widget.h"

WrappingTextEditWidget::WrappingTextEditWidget(
	QWidget *parent
)
	: QTextEdit(parent), m_title("")
{
	setFrameStyle(QFrame::NoFrame);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAcceptRichText(false);

	document()->setDocumentMargin(0);
	document()->setIndentWidth(0);

	setFocusPolicy(Qt::NoFocus);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
}

bool WrappingTextEditWidget::hasHeightForWidth() const {
	return true;
}

int WrappingTextEditWidget::heightForWidth(int width) const {
	QFont font = currentFont();
	QFontMetrics metrics(font);
	QString text = toPlainText();

	// Calculate bounding rect for the text.
	QRect bounds = metrics.boundingRect(
		QRect(0, 0, width, INT_MAX),
		Qt::AlignLeft | Qt::TextWordWrap,
		text
	);
	QSize textSize = bounds.size();
	return textSize.height();
}

void WrappingTextEditWidget::setTitle(const QString& title) {
	m_title = title;
	setText(title);
}

QString& WrappingTextEditWidget::getTitle() {
	return m_title;
}

// Events.

void WrappingTextEditWidget::enterEvent(QEnterEvent*) {
	// TODO: Is there a better way to do this?
	QGuiApplication::setOverrideCursor(isReadOnly() ? Qt::ArrowCursor : Qt::IBeamCursor);
}

void WrappingTextEditWidget::leaveEvent(QEvent*) {
	// TODO: Is there a better way to do this?
	QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
}

void WrappingTextEditWidget::keyPressEvent(QKeyEvent *event) {
	if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
		QString newTitle = toPlainText();
		emit editConfirmed(newTitle, [this, newTitle](bool result){
			if (result) {
				this->clearFocus();
				this->setTitle(newTitle);
			}
		});
	} else if (event->key() == Qt::Key_Escape) {
		cancel();
	} else if (QString text = event->text(); !text.isEmpty()) {
		QString currentText = toPlainText();
		if (
			(currentText.length() + text.length() > maxLength) &&
			(text.length() > 1 || text[0].isPrint())
		) {
			return;
		} else {
			QTextEdit::keyPressEvent(event);
			updateGeometry();
		}
	} else {
		QTextEdit::keyPressEvent(event);
		updateGeometry();
	}
}

void WrappingTextEditWidget::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		if (!hasFocus()) {
			setFocus();
		}
		QTextEdit::mousePressEvent(event);
	}
}

void WrappingTextEditWidget::focusOutEvent(QFocusEvent *event) {
	cancel();
	QTextEdit::focusOutEvent(event);
}

void WrappingTextEditWidget::insertFromMimeData(const QMimeData *source) {
	if (source == nullptr)
		return;

	QString text = source->text();
	QString currentText = toPlainText();
	int availableLength = maxLength - currentText.length();

	if (availableLength  > 0) {
		QString availableText = text.left(availableLength);
		QMimeData newData = QMimeData();
		newData.setText(availableText);
		QTextEdit::insertFromMimeData(&newData);
	}
}

// Utility.

void WrappingTextEditWidget::cancel() {
	clearFocus();
	setText(m_title);
}

