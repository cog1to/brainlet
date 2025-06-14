#include <QFrame>
#include <QSize>
#include <QString>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QPainter>

#include "model/thought.h"
#include "widgets/history_widget.h"

// History list widget.

HistoryWidget::HistoryWidget(QWidget *parent, Style *style)
	: QFrame(parent), m_style(style)
{
	setContentsMargins(QMargins(0, 0, 0, 0));
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

HistoryWidget::~HistoryWidget() {
	for (int idx = 0; idx < m_items.size(); idx++) {
		delete m_items[idx];
	}
}

QSize HistoryWidget::sizeHint() const {
	QFontMetrics metrics = QFontMetrics(m_style->historyFont());
	return QSize(100, metrics.height() + 8);
}

void HistoryWidget::resizeEvent(QResizeEvent*) {
	relayout();
}

void HistoryWidget::addItem(ThoughtId id, QString& title) {
	for (int idx = 0; idx < m_items.size(); idx++) {
		HistoryItem *w = m_items[idx];
		if (w->id() == id) {
			m_items.removeAt(idx);
			m_items.insert(0, w);
			relayout();
			return;
		}
	}

	HistoryItem *item = new HistoryItem(this, m_style, id, title);
	m_items.insert(0, item);

	// Connect.
	connect(
		item, SIGNAL(clicked(HistoryItem*)),
		this, SLOT(onItemClicked(HistoryItem*))
	);

	// Clean up if we got too many widgets.
	while (m_items.count() > 20) {
		HistoryItem *lastWidget = m_items[m_items.count() - 1];
		m_items.removeAt(m_items.count() - 1);
		delete lastWidget;
	}

	// Trigger redraw.
	relayout();
	update();
}

void HistoryWidget::onItemClicked(HistoryItem *item) {
	emit itemSelected(item->id(), item->name());
}

void HistoryWidget::relayout() {
	int spacing = 0, vcount = 0, layoutSpacing = 6;
	int offset = size().width();
	QMargins margins = contentsMargins();
	int availableWidth = size().width() - margins.left() - margins.right();

	// Calculate the number of visible items.
	int consumedWidth = 0;
	for (vcount = 0; vcount < m_items.size(); vcount++) {
		int w = m_items[vcount]->sizeHint().width() + spacing;
		if (consumedWidth + w > availableWidth)
			break;
		consumedWidth += w;
		spacing = layoutSpacing;
	}

	// Make everyting that fits visible.
	for (int idx = 0; idx < vcount; idx++) {
		QWidget *item = m_items[idx];
		QSize hint = item->sizeHint();

		item->setParent(this);
		item->show();
		item->setGeometry(
			offset - hint.width(), 0,
			hint.width(), hint.height()
		);
		offset -= hint.width() + spacing;
	}

	// Hide everything that is not fit.
	for (int idx = vcount; idx < m_items.size(); idx++)
		m_items[idx]->setParent(nullptr);
}

// History item.

HistoryItem::HistoryItem(
	QWidget *parent,
	Style *style,
	ThoughtId id,
	QString& name
)
	: QFrame(parent), m_style(style), m_name(name), m_id(id)
{
	setStyleSheet(
		"QFrame{\
			background-color: #74000000;\
			border-radius: 4px;\
		}\
		QFrame::hover{\
			background-color: #74343434;\
		}"
	);

	setContentsMargins(QMargins(6, 4, 6, 4));
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
}

QSize HistoryItem::sizeHint() const {
	QFontMetrics metrics = QFontMetrics(m_style->historyFont());
	QRect textRect = metrics.boundingRect(m_name);
	QMargins margins = contentsMargins();

	return QSize(
		std::min(textRect.width() + margins.left() + margins.right(), 100),
		metrics.height() + margins.top() + margins.bottom()
	);
}

void HistoryItem::paintEvent(QPaintEvent *event) {
	QFrame::paintEvent(event);

	QFontMetrics metrics = QFontMetrics(m_style->historyFont());
	QMargins margins = contentsMargins();
	QSize cur = size();
	int availableWidth = cur.width() - margins.left() - margins.right();

	QPainter painter(this);
	painter.setFont(m_style->historyFont());
	painter.setPen(m_style->textColor());

	QRect textRect = metrics.boundingRect(m_name);
	QString text = m_name;

	if (textRect.width() > availableWidth) {
		text = metrics.elidedText(
			m_name,
			Qt::ElideRight,
			availableWidth
		);
	}

	painter.drawText(
		QPointF(margins.left(), margins.top() + metrics.ascent()),
		text
	);
}

void HistoryItem::mouseReleaseEvent(QMouseEvent *ev) {
	QPoint pos = ev->pos();
	if (pos.x() >= 0 && pos.x() <= size().width() && pos.y() >= 0 && pos.y() <= size().height())
		emit clicked(this);
}

QString& HistoryItem::name() {
	return m_name;
}

ThoughtId HistoryItem::id() {
	return m_id;
}
