#include <QColor>
#include <QPainter>
#include <QPen>
#include <QWidget>

#include "layout/scroll_area_layout.h"
#include "widgets/scroll_bar_widget.h"

ScrollBarWidget::ScrollBarWidget(
	QWidget* parent,
	Style* style,
	ScrollBarPos pos
)
	: BaseWidget(parent, style)
{
	m_pos = pos;	
}

void ScrollBarWidget::setSettings(float width, float offset) {
	m_barWidth = width;
	m_offset = offset;
	update();
}

void ScrollBarWidget::paintEvent(QPaintEvent *) {
	QPainter painter(this);
	QSize cur = size();
	float width, offset;

	QColor background = m_style->background().darker();
	QColor foreground = m_style->background().lighter(200);
	int margin = (float)(std::min(size().width(), size().height())) / 2.0;

	QPen backgroundPen(background, margin, Qt::SolidLine, Qt::RoundCap);
	painter.setPen(backgroundPen);
	painter.drawLine(margin, margin, size().width() - margin, size().height() - margin);

	QPen foregroundPen(foreground, margin, Qt::SolidLine, Qt::RoundCap);
	painter.setPen(foregroundPen);

	if (m_pos == ScrollBarPos::Left || m_pos == ScrollBarPos::Right) {
		width = ((float)(cur.height() - margin * 2) * m_barWidth);
		offset = ((cur.height() - margin * 2) * m_offset);

		painter.drawLine(
			margin,
			margin + offset,
			margin,
			margin + offset + width
		);
	} else {
		width = ((float)(cur.width() - margin * 2) * m_barWidth);
		offset = ((cur.width() - margin * 2) * m_offset);

		painter.drawLine(
			margin + offset,
			margin,
			margin + offset + width,
			margin
		);
	}


#ifdef DEBUG_GUI
	QPen areaPen(QColor(0, 0, 0, 255));
	QList<qreal> dashes; dashes << 4 << 4;
	areaPen.setDashPattern(dashes);
	painter.setBrush(Qt::NoBrush);
	painter.setPen(areaPen);
	painter.drawRect(1, 1, cur.width() - 2, cur.height() - 2);
#endif
}

