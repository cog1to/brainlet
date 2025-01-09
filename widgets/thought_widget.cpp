#include <QTextEdit>
#include <QString>
#include <QPainter>

#include "widgets/thought_widget.h"

ThoughtWidget::ThoughtWidget(
	QWidget *parent,
	Style *style,
	std::string text,
	bool hasParent,
	bool hasChild,
	bool hasLink
): BaseWidget(parent, style),
	m_anchorLink(this, style, hasLink),
	m_anchorParent(this, style, hasParent),
	m_anchorChild(this, style, hasChild),
	m_textEdit(this, style, text)
{
	QObject::connect(
		&m_textEdit, &ThoughtEditWidget::mouseEnter,
		this, &ThoughtWidget::onTextEnter
	);
	QObject::connect(
		&m_textEdit, &ThoughtEditWidget::mouseLeave,
		this, &ThoughtWidget::onTextLeave
	);
}

ThoughtWidget::~ThoughtWidget() {}

const bool ThoughtWidget::hasParent() const {
	return m_anchorParent.active();
}

void ThoughtWidget::setHasParent(bool value) {
	m_anchorParent.setActive(value);
}

const bool ThoughtWidget::hasChild() const {
	return m_anchorChild.active();
}

void ThoughtWidget::sethHasChild(bool value) {
	m_anchorChild.setActive(value);
}

const bool ThoughtWidget::hasLink() const {
	return m_anchorLink.active();
}

void ThoughtWidget::setHasLink(bool value) {
	m_anchorLink.setActive(value);
}

const std::string ThoughtWidget::text() const {
	return m_textEdit.toPlainText().toStdString();
}

void ThoughtWidget::setText(std::string value) {
	m_textEdit.setPlainText(QString::fromStdString(value));
}

// Size hint

QSize ThoughtWidget::sizeHint() const {
	const QSize anchorSize = AnchorWidget::defaultSize;

	// Calculate bounding rect for the text.
	QFontMetrics metrics(m_style->font());
	QRect bounds = metrics.boundingRect(m_textEdit.toPlainText());
	// Update desired page size for the text edit.
	m_textEdit.document()->setPageSize(QSizeF(bounds.width(), bounds.height()));
	// Get actual text size from the document.
	QSize textSize = m_textEdit.document()->size().toSize();
	// Add paddings and anchor sizes and borders.
	QSize result(
		textSize.width() + padding.width() * 2.0 + m_style->hoverBorderWidth() * 1.5 + anchorSize.width() / 2.0,
		textSize.height() + padding.height() * 2.0 + m_style->hoverBorderWidth() * 2.0 + anchorSize.height()
	);

	return result;
}

// Hover event

void ThoughtWidget::onTextEnter() {
	m_hover = true;
	update();
}

void ThoughtWidget::onTextLeave() {
	m_hover = false;
	update();
}

// Draw and layout

void ThoughtWidget::paintEvent(QPaintEvent *event) {
	const QSize anchorSize = AnchorWidget::defaultSize;

	float borderWidth = m_style->borderWidth();
	QColor borderColor = m_style->borderColor();
	QColor hoverColor = m_style->hoverBorderColor();
	float hoverWidth = m_style->hoverBorderWidth();
	QSize cur = size();
	QPen pen(borderColor, borderWidth);
	if (m_hover) {
		pen.setWidth(hoverWidth);
	}
	QPainter painter(this);

	QRectF border(
		anchorSize.width() / 2.0,
		anchorSize.height() / 2.0,
		cur.width() - hoverWidth/2.0 - anchorSize.width()/2.0,
		cur.height() - hoverWidth/2.0 - anchorSize.height());

	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(pen);
	painter.drawRoundedRect(border, 12, 12);

#ifdef DEBUG_GUI
	QPen areaPen(QColor(0, 0, 0, 255));
	QList<qreal> dashes; dashes << 4 << 4;
	areaPen.setDashPattern(dashes);
	painter.setBrush(Qt::NoBrush);
	painter.setPen(areaPen);
	painter.drawRect(1, 1, cur.width() - 2, cur.height() - 2);
#endif
}

void ThoughtWidget::resizeEvent(QResizeEvent *event) {
	const QSize anchorSize = AnchorWidget::defaultSize;
	const qreal parentLeftOffset = 0.65;
	const qreal childLeftOffset = 0.3;
	const QSize size = event->size();

	QRect parent(
		(int)((float)size.width() * parentLeftOffset),
		0,
		anchorSize.width(), anchorSize.height());
	m_anchorParent.setGeometry(parent);

	QRect child(
		(int)((float)size.width() * childLeftOffset),
		size.height() - anchorSize.width() - m_style->hoverBorderWidth() / 2.0,
		anchorSize.width(), anchorSize.height());
	m_anchorChild.setGeometry(child);

	QRect link(
		0,
		(int)((float)(size.height() - anchorSize.height()) / 2.0),
		anchorSize.width(), anchorSize.height());
	m_anchorLink.setGeometry(link);

	QRect text(
		anchorSize.width() / 2.0 + m_style->hoverBorderWidth() / 2.0 + padding.width(),
		anchorSize.height() / 2.0 + m_style->hoverBorderWidth() / 2.0 + padding.height(),
		size.width() - anchorSize.width() / 2.0 - m_style->hoverBorderWidth() * 1.5 - padding.width() * 2,
		size.height() - anchorSize.height() - m_style->hoverBorderWidth() - padding.height() * 2);
	m_textEdit.setGeometry(text);
}

