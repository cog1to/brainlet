#include <QWidget>
#include <QScrollArea>
#include <QLine>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QMargins>

#include "widgets/markdown_scroll_widget.h"
#include "widgets/markdown_edit_widget.h"

#include <QDebug>

MarkdownScrollWidget::MarkdownScrollWidget(
	QWidget *parent,
	Style *style
) : QScrollArea(parent),
	m_style(style),
	m_container(nullptr),
	m_layout(nullptr)
{
	setStyleSheet("QScrollArea{border:none;}");
	setFocusPolicy(Qt::NoFocus);
	m_layout.setContentsMargins(QMargins(0, 0, 0, 0));
	m_container.setLayout(&m_layout);
	m_container.setFocusPolicy(Qt::NoFocus);
	setWidget(&m_container);
}

MarkdownScrollWidget::~MarkdownScrollWidget() {};

void MarkdownScrollWidget::setMarkdownWidgets(
	MarkdownEditWidget *w,
	MarkdownConnectionsWidget *c
) {
	assert(w != nullptr);

	connect(
		w, &MarkdownEditWidget::cursorMoved,
		this, &MarkdownScrollWidget::onCursorMoved
	);

	m_widget = w;
	m_widget->setPresenter(this);
	m_layout.addWidget(w);

	if (c != nullptr) {
		m_connections = c;
		m_layout.addWidget(c);
	}
}

MarkdownEditWidget *MarkdownScrollWidget::markdownWidget() {
	return m_widget;
}

void MarkdownScrollWidget::onCursorMoved(QLine line, bool up) {
	if (up) {
		ensureVisible(line.x1(), line.y1(), 10, 0);
	} else {
		ensureVisible(line.x2(), line.y2(), 10, 0);
	}
}

int MarkdownScrollWidget::getPageOffset(bool down) {
	QRect viewportRect = viewport()->rect();
	QScrollBar *bar = verticalScrollBar();

	int oldValue = bar->value();
	int newValue = down
		? std::min(bar->maximum(), bar->value() + bar->pageStep())
		: std::max(0, bar->value() - bar->pageStep());

	// Calculate new position of the viewport.
	int yDiff = float(newValue - oldValue)
		/ (float)(bar->maximum() + bar->pageStep()) 
		* m_widget->rect().height();

	return yDiff;
}

// Slots.

void MarkdownScrollWidget::onError(MarkdownScrollError error) {
	if (m_error != nullptr)
		delete m_error;

	if (error == MarkdownScrollIOError) {
		m_error = new ToastWidget(m_style, tr("Failed to access file system"));
		m_error->show(this);
	}
}

