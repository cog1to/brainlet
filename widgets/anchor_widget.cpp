#include "widgets/anchor_widget.h"

#include <QPainter>
#include <QWidget>
#include <QBrush>

AnchorWidget::AnchorWidget(QWidget *parent, Style *style, bool active)
	: BaseWidget(parent, style)
{
	m_active = active;
}

AnchorWidget::~AnchorWidget() {}

const bool AnchorWidget::active() const {
	return m_active;
}

void AnchorWidget::setActive(bool active) {
	m_active = active;
	update();
}

QSize AnchorWidget::sizeHint() const {
	return QSize(24, 24);
}

void AnchorWidget::enterEvent(QEnterEvent*) {
	emit mouseEnter(this);
}

void AnchorWidget::leaveEvent(QEvent*) {
	emit mouseLeave(this);
}

void AnchorWidget::paintEvent(QPaintEvent *) {
	float borderWidth = m_style->borderWidth();
	QColor background = m_style->background();
	QColor active = m_style->activeAnchorColor();
	QColor border = m_style->borderColor();
	QSize cur = size();
	QBrush brush = QBrush(background);
	QPen pen(border, borderWidth);
	QPainter painter(this);
	QRectF circle((cur.width() - 8.0) / 2.0, (cur.height() - 8.0) / 2.0, 8.0, 8.0);

	painter.setRenderHint(QPainter::Antialiasing);

	if (m_active) {
		brush.setColor(active);
		pen.setColor(active);
	}

	painter.setBrush(brush);
	painter.setPen(pen);
	painter.drawEllipse(circle);

#ifdef DEBUG_GUI
	QPen areaPen(QColor(0, 0, 0, 255));
	QList<qreal> dashes; dashes << 4 << 4;
	areaPen.setDashPattern(dashes);
	painter.setBrush(Qt::NoBrush);
	painter.setPen(areaPen);
	painter.drawRect(1, 1, cur.width() - 2, cur.height() - 2);
#endif
}
