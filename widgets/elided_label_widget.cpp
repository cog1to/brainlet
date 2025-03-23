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
	setFrameStyle(QFrame::Box);
}

QSize ElidedLabelWidget::sizeHint() const {
	QFontMetrics metrics(font());
	QRect bounds = metrics.boundingRect(m_text);
	QSize textSize = bounds.size();
	QMargins margins = contentsMargins();

	return QSize(
		textSize.width() + margins.left() + margins.right(),
		textSize.height() + margins.top() + margins.bottom()
	);
}

void ElidedLabelWidget::paintEvent(QPaintEvent *event) {
	QFrame::paintEvent(event);

	QPainter painter(this);
	QFontMetrics fontMetrics = painter.fontMetrics();

	QMargins margins = contentsMargins();
	int availableWidth = width() - margins.left() - margins.right();
	int availableHeight = height() - margins.bottom();
	int y = margins.top();
	int lineSpacing = fontMetrics.lineSpacing();

	QTextLayout textLayout(m_text, painter.font());
	textLayout.beginLayout();

	forever {
		QTextLine line = textLayout.createLine();

		if (!line.isValid())
				break;

		line.setLineWidth(availableWidth);
		int nextLineY = y + lineSpacing;
		int y = margins.top();

		if (availableHeight >= nextLineY + lineSpacing) {
			line.draw(&painter, QPoint(margins.left(), y));
			y = nextLineY;
		} else {
			QString lastLine = m_text.mid(line.textStart());
			QString elidedLastLine = fontMetrics.elidedText(
				lastLine,
				Qt::ElideRight,
				availableWidth
			);
			painter.drawText(
				QPoint(margins.left(), y + fontMetrics.ascent()),
				elidedLastLine
			);
			line = textLayout.createLine();
			break;
		}
	}

	textLayout.endLayout();
}

