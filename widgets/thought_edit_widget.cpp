#include <QColor>

#include "widgets/thought_edit_widget.h"
#include "widgets/style.h"

ThoughtEditWidget::ThoughtEditWidget(QWidget *parent, Style *style, std::string text)
	: QTextEdit(parent)
{
	setReadOnly(true);
	setFrameStyle(QFrame::NoFrame);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QString stylesheet = QString("background-color: #00000000; padding: 0; color: %1; font: %2 %3pt \"%4\";")
			.arg(style->textColor().name(QColor::HexRgb))
			.arg(style->font().bold() ? "bold" : "")
			.arg(style->font().pointSize())
			.arg(style->font().family());
	setStyleSheet(stylesheet);

	document()->setDocumentMargin(0);
	document()->setIndentWidth(0);
	setPlainText(QString::fromStdString(text));
	setAlignment(Qt::AlignCenter);
}

void ThoughtEditWidget::enterEvent(QEnterEvent*) {
	emit mouseEnter();
}

void ThoughtEditWidget::leaveEvent(QEvent*) {
	emit mouseLeave();
}

