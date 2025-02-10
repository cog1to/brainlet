#include <QPainter>
#include <QWidget>
#include <QBrush>
#include <QMouseEvent>

#include "widgets/anchor_widget.h"

#include <QDebug>

AnchorWidget::AnchorWidget(
	QWidget *parent,
	Style *style,
	AnchorType type,
	bool active
)
	: BaseWidget(parent, style)
{
	m_active = active;
	m_pressed = false;
	m_type = type;
}

AnchorWidget::~AnchorWidget() {}

const bool AnchorWidget::active() const {
	return m_active;
}

void AnchorWidget::setActive(bool active) {
	m_active = active;
	update();
}

const AnchorType AnchorWidget::type() const {
	return m_type;
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

void AnchorWidget::mousePressEvent(QMouseEvent* event) {
	m_pressed = true;
	//grabKeyboard();
	m_dragStart = event->pos();
}

void AnchorWidget::mouseReleaseEvent(QMouseEvent* event) {
	m_pressed = false;

	QPoint point = event->pos();
	QSize current = size();

	emit mouseRelease(
		this,
		QPoint(
			current.width() / 2 + point.x() - m_dragStart.x(),
			current.height() / 2 + point.y() - m_dragStart.y()
		)
	);

	// If we're still inside the anchor, reactivate.
	if (
		point.x() >= 0 && point.x() <= current.width() &&
		point.y() >= 0 && point.y() <= current.height()
	) {
		emit mouseEnter(this);
	}
}

void AnchorWidget::mouseMoveEvent(QMouseEvent *event) {
	if (!m_pressed)
		return;

	QPoint point = event->pos();
	QSize current = size();

	emit mouseMove(
		this,
		QPoint(
			current.width() / 2 + point.x() - m_dragStart.x(),
			current.height() / 2 + point.y() - m_dragStart.y()
		)
	);
}

void AnchorWidget::keyPressEvent(QKeyEvent *event) {
	if (m_pressed && event->key() == Qt::Key_Escape) {
		m_pressed = false;
		releaseKeyboard();
		emit mouseCancel(this);
	}
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

	if (m_active || m_pressed) {
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
