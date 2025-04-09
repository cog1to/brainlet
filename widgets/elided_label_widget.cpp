#include <QFrame>
#include <QLabel>
#include <QString>
#include <QMargins>
#include <QPaintEvent>
#include <QSizePolicy>
#include <QTextLayout>
#include <QPainter>

#include "widgets/elided_label_widget.h"

ElidedLabelWidget::ElidedLabelWidget(QWidget *parent, QString text)
	: QFrame(parent), m_text(text)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	setContentsMargins(QMargins(0, 0, 0, 0));
}

QSize ElidedLabelWidget::sizeHint() const {
	QFontMetrics metrics(font());
	QRect bounds = metrics.boundingRect(m_text);
	QSize textSize = bounds.size();
	QMargins margins = contentsMargins();

	return QSize(
		textSize.width() + margins.left() + margins.right(),
		metrics.height() + margins.top() + margins.bottom()
	);
}

void ElidedLabelWidget::setText(QString text) {
	m_text = text;
}

void ElidedLabelWidget::paintEvent(QPaintEvent *event) {
	QFrame::paintEvent(event);

	QPainter painter(this);
	QFontMetrics fontMetrics = painter.fontMetrics();

	QMargins margins = contentsMargins();
	int availableWidth = width() - margins.left() - margins.right();
	int availableHeight = height() - margins.bottom() - margins.top();
	int fontHeight = fontMetrics.height();
	int y = margins.top() + (availableHeight - fontHeight) / 2;

	QString elidedLine = fontMetrics.elidedText(
		m_text,
		Qt::ElideRight,
		availableWidth
	);
	painter.drawText(
		QPoint(margins.left(), y + fontMetrics.ascent()),
		elidedLine
	);
}

