#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

#include "widgets/anchor_highlight_widget.h"

AnchorHighlightWidget::AnchorHighlightWidget(QWidget *parent, Style *style)
	: BaseWidget(parent, style),
	  m_anchor(this, style, AnchorType::AnchorParent, true)
{
	setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void AnchorHighlightWidget::resizeEvent(QResizeEvent *event) {
	QSize newSize = event->size();
	QSize anchorSize = AnchorWidget::defaultSize;

	m_anchor.setGeometry(
		(newSize.width() - anchorSize.width()) / 2,
		(newSize.height() - anchorSize.height()) / 2,
		anchorSize.width(),
		anchorSize.height()
	);
}

void AnchorHighlightWidget::paintEvent(QPaintEvent*) {
	QColor color = m_style->anchorHighlight();
	QSize current = size();

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	painter.setPen(QPen(color, 1));
	painter.drawEllipse(1, 1, current.width() - 2, current.height() - 2);
}
