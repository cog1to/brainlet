#include <QFrame>
#include <QSize>
#include <QString>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QPainter>

#include "model/thought.h"
#include "widgets/history_widget.h"

HistoryWidget::HistoryWidget(QWidget *parent, Style *style)
	: QFrame(parent), m_style(style), m_layout(this)
{
	setContentsMargins(QMargins(0, 0, 0, 0));
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_layout.setSpacing(6);
	m_layout.setContentsMargins(QMargins(0, 0, 0, 0));
	m_layout.setDirection(QBoxLayout::RightToLeft);
	m_layout.addStretch(0);
}

HistoryWidget::~HistoryWidget() {
	for (int idx = 0; idx < m_items.size(); idx++) {
		delete m_items[idx];
	}
}

QSize HistoryWidget::sizeHint() const {
	QFontMetrics metrics = QFontMetrics(m_style->historyFont());
	return QSize(100, metrics.height() + 4);
}

void HistoryWidget::addItem(ThoughtId id, QString& title) {
	for (int idx = 0; idx < m_layout.count(); idx++) {
		auto item = m_layout.itemAt(idx);
		if (item->widget() != nullptr) {
			HistoryItem *w = static_cast<HistoryItem*>(item->widget());
			if (w->id() == id) {
				m_layout.removeItem(item);
				m_layout.insertWidget(0, w);
				return;
			}
		}
	}

	HistoryItem *item = new HistoryItem(nullptr, m_style, id, title);
	m_layout.insertWidget(0, item);
	m_items.push_back(item);

	// Connect.
	connect(
		item, SIGNAL(clicked(HistoryItem*)),
		this, SLOT(onItemClicked(HistoryItem*))
	);

	// Clean up if we got too many widgets.
	while (m_layout.count() > 10) {
		HistoryItem *lastWidget = nullptr;
		QLayoutItem *lastItem = nullptr;

		// Find last history item.
		for (int iidx = m_layout.count() - 1; iidx >= 0; iidx--) {
			auto item = m_layout.itemAt(iidx);
			if (item->widget() != nullptr) {
				HistoryItem *it = static_cast<HistoryItem*>(item->widget());
				if (it != nullptr) {
					lastWidget = it;
					lastItem = item;
					break;
				}
			}
		}

		if (lastWidget != nullptr && lastItem != nullptr) {
			// Delete from list.
			for (int idx = 0; idx < m_items.size(); idx++) {
				if (m_items[idx] == lastWidget) {
					m_items.removeAt(idx);
					delete lastWidget;
					break;
				}
			}
			m_layout.removeItem(lastItem);
		} else {
			break;
		}
	}

	// Trigger redraw.
	update();
}

void HistoryWidget::onItemClicked(HistoryItem *item) {
	emit itemSelected(item->id(), item->name());
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
			background-color: #74444444;\
		}"
	);

	setContentsMargins(QMargins(6, 2, 6, 2));
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
