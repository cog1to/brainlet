#include <QTextEdit>
#include <QString>
#include <QPainter>
#include <iostream>

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
	m_textEdit(this, style, "")
{
	m_text = QString::fromStdString(text);

	QObject::connect(
		&m_textEdit, &ThoughtEditWidget::mouseEnter,
		this, &ThoughtWidget::onTextEnter
	);
	QObject::connect(
		&m_textEdit, &ThoughtEditWidget::mouseLeave,
		this, &ThoughtWidget::onTextLeave
	);

	updateText();
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
	return m_text.toStdString();
}

void ThoughtWidget::setText(std::string value) {
	m_text = QString::fromStdString(value);
	updateText();
}

// Size measurements

QSize ThoughtWidget::sizeHint() const {
	const QSize anchorSize = AnchorWidget::defaultSize;

	// Calculate bounding rect for the text.
	QFontMetrics metrics(m_style->font());
	QRect bounds = metrics.boundingRect(m_text);
	QSize textSize = bounds.size();
	// Add paddings and anchor sizes and borders.
	QSize result(
		textSize.width() + padding.width() * 2.0 + m_style->hoverBorderWidth() * 2.0 + anchorSize.width(),
		textSize.height() + padding.height() * 2.0 + m_style->hoverBorderWidth() * 2.0 + anchorSize.height()
	);

	return result;
}

QSize ThoughtWidget::sizeForWidth(int width) const {
	const QSize anchorSize = AnchorWidget::defaultSize;
	const int textPadding = anchorSize.width() + padding.width() * 2.0 + m_style->hoverBorderWidth();
	const int verticalPadding = anchorSize.height() + padding.height() * 2.0 + m_style->hoverBorderWidth();

	QFontMetrics metrics(m_style->font());
	QRect bounds = metrics.boundingRect(
		QRect(0, 0, width - textPadding, INT_MAX),
		Qt::AlignHCenter | Qt::TextWordWrap,
		m_text
	);

	return QSize(
		bounds.size().width() + textPadding,
		// TODO: Figure out how to properly measure text height. This is a hack to
		// take into account line spacing added by the QTextEdit.
		bounds.size().height() + verticalPadding + ((bounds.height() / metrics.height()))
	);
}

// Hover event

void ThoughtWidget::onTextEnter() {
	m_hover = true;
	update();
	emit textMouseEnter(this);
}

void ThoughtWidget::onTextLeave() {
	m_hover = false;
	update();
	emit textMouseLeave(this);
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

	QPainter painter(this);

	if (m_hover) {
		pen.setWidth(hoverWidth);
		QBrush brush(m_style->hoverBackground());
		painter.setBrush(brush);
	}

	QRectF border(
		anchorSize.width() / 2.0,
		anchorSize.height() / 2.0,
		cur.width() - hoverWidth/2.0 - anchorSize.width(),
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
		size.width() - anchorSize.width() - m_style->hoverBorderWidth() * 2 - padding.width() * 2,
		size.height() - anchorSize.height() - m_style->hoverBorderWidth() - padding.height() * 2);
	m_textEdit.setGeometry(text);

	updateText();
}

// Private

void ThoughtWidget::updateText() {
	QFontMetrics metrics(m_style->font());

	const QSize anchorSize = AnchorWidget::defaultSize;
	const int textPadding = anchorSize.width() + padding.width() * 2.0 + m_style->hoverBorderWidth();
	const int verticalPadding = anchorSize.height() + padding.height() * 2.0 + m_style->hoverBorderWidth();
	const int availableWidth = size().width() - textPadding;
	QRect bounds = metrics.boundingRect(
		QRect(0, 0, size().width() - textPadding, INT_MAX),
		Qt::AlignHCenter | Qt::TextWordWrap,
		m_text
	);

	if (bounds.size().height() > (size().height() - verticalPadding)) {
		QString elided = metrics.elidedText(m_text, Qt::ElideRight, availableWidth);
		m_textEdit.setPlainText(elided);
	} else {
		m_textEdit.setPlainText(m_text);
	}

	m_textEdit.setAlignment(Qt::AlignCenter);
}
