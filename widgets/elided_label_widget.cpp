#include <QFrame>
#include <QLabel>
#include <QString>
#include <QMargins>
#include <QPaintEvent>
#include <QSizePolicy>
#include <QTextLayout>
#include <QPainter>

#include <QDebug>

#include "widgets/elided_label_widget.h"

ElidedLabelWidget::ElidedLabelWidget(QWidget *parent, QString text, bool multiline)
	: QFrame(parent), m_text(text), m_multiline(multiline)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setContentsMargins(QMargins(0, 0, 0, 0));
}

QSize ElidedLabelWidget::sizeHint() const {
	QFontMetrics metrics(font());
	QRect bounds = metrics.boundingRect(m_text);
	QSize textSize = bounds.size();
	QMargins margins = contentsMargins();

	return QSize(
		textSize.width() + margins.left() + margins.right() + 6,
		metrics.height() + margins.top() + margins.bottom()
	);
}

bool ElidedLabelWidget::hasHeightForWidth() const {
	return m_multiline;
}

int ElidedLabelWidget::heightForWidth(int width) const {
	QFontMetrics metrics(font());

	QMargins margins = contentsMargins();
	int availableWidth = this->width() - margins.left() - margins.right();

	// Calculate bounding rect for the text.
	QRect bounds = metrics.boundingRect(
		QRect(0, 0, availableWidth, INT_MAX),
		Qt::AlignHCenter | Qt::TextWordWrap,
		m_text
	);
	QSize textSize = bounds.size();

	return textSize.height() + margins.top() + margins.bottom();
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
	const int lineSpacing = fontHeight;

	if (m_multiline) {
		int y = 0;
		QTextLayout textLayout(m_text, painter.font());
		textLayout.beginLayout();

		while (true) {
			QTextLine line = textLayout.createLine();

      if (!line.isValid())
        break;

      line.setLineWidth(availableWidth);
      int nextLineY = y + lineSpacing;

      if (height() >= nextLineY + lineSpacing) {
        line.draw(&painter, QPoint((availableWidth - line.naturalTextWidth()) / 2, y));
        y = nextLineY;
			} else {
				QString lastLine = m_text.mid(line.textStart());
				QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, availableWidth);

				// To properly center the line, we have to measure it first.
				QRect bounds = fontMetrics.boundingRect(
					QRect(0, 0, availableWidth, INT_MAX),
					Qt::AlignHCenter | Qt::TextWordWrap,
					elidedLastLine
				);

				painter.drawText(
					QPoint((availableWidth - bounds.width()) / 2, 
					y + fontMetrics.ascent()), elidedLastLine
				);

				line = textLayout.createLine();
				break;
			}
		}
		textLayout.endLayout();
	} else {
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
}

