#include <assert.h>

#include <QTextEdit>
#include <QString>
#include <QPainter>

#include "widgets/thought_widget.h"

ThoughtWidget::ThoughtWidget(
	QWidget *parent,
	Style *style,
	ThoughtId id,
	bool readOnly,
	std::string text,
	bool hasParent,
	bool hasChild,
	bool hasLink
): BaseWidget(parent, style),
	m_anchorLink(this, style, AnchorType::Link, hasLink),
	m_anchorParent(this, style, AnchorType::Parent, hasParent),
	m_anchorChild(this, style, AnchorType::Child, hasChild),
	m_textEdit(this, style, readOnly, "")
{
	m_id = id;
	m_text = QString::fromStdString(text);

	QObject::connect(
		&m_textEdit, SIGNAL(mouseEnter()),
		this, SLOT(onTextEnter())
	);
	QObject::connect(
		&m_textEdit, SIGNAL(mouseLeave()),
		this, SLOT(onTextLeave())
	);
	QObject::connect(
		&m_textEdit, SIGNAL(focusLost()),
		this, SLOT(onTextClearFocus())
	);
	QObject::connect(
		&m_textEdit, SIGNAL(textChanged()),
		this, SLOT(onTextChanged())
	);
	QObject::connect(
		&m_textEdit, SIGNAL(editStarted()),
		this, SLOT(onTextEdit())
	);
	QObject::connect(
		&m_textEdit, SIGNAL(editCanceled()),
		this, SLOT(onTextCancel())
	);
	QObject::connect(
		&m_textEdit, SIGNAL(editConfirmed(std::function<void(bool)>)),
		this, SLOT(onTextConfirmed(std::function<void(bool)>))
	);

	AnchorWidget *anchors[] = {&m_anchorLink, &m_anchorParent, &m_anchorChild};
	for (AnchorWidget *widget: anchors) {
		QObject::connect(
			widget, SIGNAL(mouseEnter(AnchorWidget*)),
			this, SLOT(onAnchorEntered(AnchorWidget*))
		);
		QObject::connect(
			widget, SIGNAL(mouseLeave(AnchorWidget*)),
			this, SLOT(onAnchorLeft(AnchorWidget*))
		);
		QObject::connect(
			widget, SIGNAL(mouseMove(AnchorWidget*, QPoint)),
			this, SLOT(onAnchorMove(AnchorWidget*, QPoint))
		);
	}

	updateText();
}

ThoughtWidget::~ThoughtWidget() {}

const ThoughtId ThoughtWidget::id() const {
	return m_id;
}

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

const bool ThoughtWidget::readOnly() const {
	return m_textEdit.isReadOnly();
}

void ThoughtWidget::setReadOnly(bool value) {
	m_textEdit.setReadOnly(value);
}

const bool ThoughtWidget::isActive() const {
	return m_hover || m_textEdit.hasFocus();
}

// Anchor coordinates.

AnchorPoint ThoughtWidget::getAnchorFrom(ConnectionType type) {
	const QSize anchorSize = AnchorWidget::defaultSize;

	QPointF point;
	switch (type) {
		case ConnectionType::child:
			point = mapToParent(QPointF(
				(float)size().width() * childLeftOffset + (float)anchorSize.width() / 2.0,
				(float)size().height() - (float)anchorSize.height() / 2.0
			));
			return AnchorPoint {
				.x = point.x(), .y = point.y(),
				.dx = 0, .dy = 1.0
			};
		case ConnectionType::link:
			point = mapToParent(QPointF(
				(float)anchorSize.width() / 2.0,
				(float)size().height() / 2.0
			));
			return AnchorPoint {
				.x = point.x(), .y = point.y(),
				.dx = -1.0, .dy = 0
			};
	}

	assert(false);
	return AnchorPoint{};
}

AnchorPoint ThoughtWidget::getAnchorFrom(AnchorType type) {
	switch (type) {
		case AnchorType::Parent:
			return getAnchorTo(ConnectionType::child);
		case AnchorType::Child:
			return getAnchorFrom(ConnectionType::child);
		case AnchorType::Link:
			return getAnchorFrom(ConnectionType::link);
	}

	assert(false);
	return AnchorPoint{};
}

AnchorPoint ThoughtWidget::getAnchorTo(ConnectionType type) {
	const QSize anchorSize = AnchorWidget::defaultSize;

	QPointF point;
	switch (type) {
		case ConnectionType::child:
			point = mapToParent(QPointF(
				(float)size().width() - (float)size().width() * parentRightOffset - (float)anchorSize.width() / 2.0,
				(float)anchorSize.height() / 2.0
			));
			return AnchorPoint {
				.x = point.x(), .y = point.y(),
				.dx = 0, .dy = -1.0
			};
		case ConnectionType::link:
			point = mapToParent(QPointF(
				(float)size().width() - (float)anchorSize.width() / 2.0,
				(float)size().height() / 2.0
			));
			return AnchorPoint {
				.x = point.x(), .y = point.y(),
				.dx = 1.0, .dy = 0
			};
	}

	assert(false);
	return AnchorPoint{};
}

// Size measurements

QSize ThoughtWidget::sizeHint() const {
	const QSize anchorSize = AnchorWidget::defaultSize;

	// Calculate bounding rect for the text.
	QFontMetrics metrics(m_style->font());
	QRect bounds = metrics.boundingRect(
		// 9000 is set due to a quirk in QFontMetrics measurement. When calling
		// boundingRect() without a restriction rect or with INT_MAX, it produces
		// a bounding rect smaller than actually needed to fit the text.
		QRect(0, 0, 9000, INT_MAX),
		Qt::AlignHCenter | Qt::TextWordWrap,
		m_text
	);
	QSize textSize = bounds.size();

	// Add paddings and anchor sizes and borders.
	QSize result(
		textSize.width() +
			padding.width() * 2 +
			m_style->hoverBorderWidth() * 2 +
			anchorSize.width(),
		textSize.height() +
			padding.height() * 2 +
			m_style->hoverBorderWidth() * 2 +
			anchorSize.height()
	);

	return result;
}

QSize ThoughtWidget::sizeForWidth(int width) const {
	const QSize anchorSize = AnchorWidget::defaultSize;

	const int textPadding = anchorSize.width() +
		padding.width() * 2 +
		m_style->hoverBorderWidth() * 2 +
		1;
	const int verticalPadding = anchorSize.height() +
		padding.height() * 2 +
		m_style->hoverBorderWidth() * 2;

	QFontMetrics metrics(m_style->font());
	QRect bounds = metrics.boundingRect(
		QRect(0, 0, width - textPadding, INT_MAX),
		Qt::AlignHCenter | Qt::TextWordWrap,
		m_text
	);

	QSize result = QSize(
		bounds.size().width() + textPadding,
		bounds.size().height() + verticalPadding
	);

	return result;
}

// Text edit events

void ThoughtWidget::onTextEnter() {
	m_hover = true;
	update();
	if (!m_textEdit.hasFocus()) {
		emit activated(this);
	}
}

void ThoughtWidget::onTextLeave() {
	m_hover = false;
	update();
	if (!m_textEdit.hasFocus()) {
		emit deactivated(this);
	}
}

void ThoughtWidget::onTextClearFocus() {
	// Update text after editing.
	m_text = m_textEdit.toPlainText();
	// Redraw.
	update();
	// Signal that we're no longer active.
	if (!m_hover) {
		emit deactivated(this);
	}
}

void ThoughtWidget::onTextChanged() {
	if (!m_textEdit.hasFocus()) {
		return;
	}

	m_text = m_textEdit.toPlainText();
	// Save cursor because a resize can occur, and that will reset it.
	m_cursor = m_textEdit.textCursor();
	emit textChanged(this);
}

void ThoughtWidget::onTextEdit() {
	m_originalText = m_text;
}

void ThoughtWidget::onTextCancel() {
	m_text = m_originalText;
	m_textEdit.setPlainText(m_text);
	emit textChanged(this);
}

void ThoughtWidget::onTextConfirmed(std::function<void(bool)> callback) {
	emit textConfirmed(this, m_textEdit.toPlainText(), callback);
}

// Anchor events

void ThoughtWidget::onAnchorEntered(AnchorWidget* widget) {
	QRect rect = widget->geometry();
	QPoint center = QPoint(
		rect.x() + rect.width() / 2,
		rect.y() + rect.height() / 2
	);

	emit anchorEntered(this, widget->type(), mapTo(parentWidget(), center));
}

void ThoughtWidget::onAnchorLeft(AnchorWidget*) {
	emit anchorLeft();
}

void ThoughtWidget::onAnchorMove(AnchorWidget* widget, QPoint point) {
	emit anchorMoved(
		widget->mapTo(parentWidget(), point)
	);
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

	if (m_hover || m_textEdit.hasFocus()) {
		pen.setWidth(hoverWidth);
		QBrush brush(m_style->hoverBackground());
		painter.setBrush(brush);
	} else {
		QBrush brush(m_style->nodeBackground());
		painter.setBrush(brush);
	}

	QRectF border(
		anchorSize.width() / 2.0,
		anchorSize.height() / 2.0,
		cur.width() - hoverWidth/2.0 - anchorSize.width(),
		cur.height() + hoverWidth/2.0 - anchorSize.height());

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
	const QSize size = event->size();

	QRect parent(
		(int)((float)size.width() - (float)size.width() * parentRightOffset) - anchorSize.width(),
		0,
		anchorSize.width(), anchorSize.height());
	m_anchorParent.setGeometry(parent);

	QRect child(
		(int)((float)size.width() * childLeftOffset),
		size.height() - anchorSize.width() + m_style->hoverBorderWidth() / 2.0,
		anchorSize.width(), anchorSize.height());
	m_anchorChild.setGeometry(child);

	QRect link(
		0,
		(int)((float)(size.height() - anchorSize.height()) / 2.0),
		anchorSize.width(), anchorSize.height());
	m_anchorLink.setGeometry(link);

	QRect textRect(
		anchorSize.width() / 2 +
			m_style->hoverBorderWidth() / 2 +
			padding.width(),
		anchorSize.height() / 2 +
			m_style->hoverBorderWidth() / 2 +
			padding.height(),
		size.width() - anchorSize.width() -
			m_style->hoverBorderWidth() * 2 -
			padding.width() * 2 +
			1, // 1px for cursor.
		size.height() - anchorSize.height() -
			m_style->hoverBorderWidth() -
			padding.height() * 2
	);
	m_textEdit.setGeometry(textRect);

	if (!m_textEdit.hasFocus()) {
		updateText();
	}
}

void ThoughtWidget::wheelEvent(QWheelEvent *event) {
	emit mouseScroll(this, event);
}

// Private

void ThoughtWidget::updateText() {
	const QSize anchorSize = AnchorWidget::defaultSize;
	const int textPadding = anchorSize.width() +
		padding.width() * 2 +
		m_style->hoverBorderWidth() * 2;
	const int verticalPadding = anchorSize.height() +
		padding.height() * 2 +
		m_style->hoverBorderWidth();
	const int availableWidth = size().width() - textPadding;

	QFontMetrics metrics(m_style->font());
	QRect bounds = metrics.boundingRect(
		QRect(0, 0, size().width() - textPadding, INT_MAX),
		Qt::AlignHCenter | Qt::TextWordWrap,
		m_text
	);

	if (!isActive() && ((bounds.size().height() > (size().height() - verticalPadding)) ||
			(bounds.size().width() > availableWidth)))
	{
		QString elided = metrics.elidedText(m_text, Qt::ElideRight, availableWidth);
		m_textEdit.setPlainText(elided);
	} else {
		m_textEdit.setPlainText(m_text);
	}

	m_textEdit.setAlignment(Qt::AlignCenter);
}

