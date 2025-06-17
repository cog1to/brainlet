#include <QWidget>
#include <QTextBrowser>
#include <QSizePolicy>
#include <QColor>

#include "widgets/markdown_connections_widget.h"
#include "widgets/style.h"
#include "model/thought.h"
#include "model/connection.h"

MarkdownConnectionsWidget::MarkdownConnectionsWidget(
	QWidget *parent,
	Style *style
) : QTextBrowser(parent), m_style(style)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	setStyleSheet(
		QString("QFrame{\
			background-color: #48000000;\
			border-radius: 12px;\
			color: %1;\
			font: normal %2px \"%3\";\
			padding: %4px;\
		}")
		.arg(style->textEditColor().name(QColor::HexRgb))
		.arg(style->textEditFont().pixelSize())
		.arg(style->textEditFont().family())
		.arg(Padding)
	);

	setFocusPolicy(Qt::NoFocus);
	setOpenLinks(false);
	setOpenExternalLinks(false);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	document()->setDefaultStyleSheet(
		QString(
			"p{margin:0px;}"
			"h4{margin:0px}"
			"a{color: %1; text-decoration: none;}"
		).arg(style->borderColor().name(QColor::HexRgb))
	);

	setHidden(true);
}

QSize MarkdownConnectionsWidget::sizeHint() const {
	QSizeF docSize = document()->size();
	return QSize(
		docSize.width() + Padding * 2,
		docSize.height() + Padding * 2
	);
}

void MarkdownConnectionsWidget::setConnections(
	QList<Connection> list
) {
	QStringList lines;
	QList<Connection> parents, children, links;

	if (list.size() == 0) {
		setHidden(true);
		return;
	}

	for (auto it = list.begin(); it != list.end(); it++) {
		if ((*it).dir() == ConnParent) {
			parents.push_back(*it);
		} else if ((*it).dir() == ConnLink) {
			links.push_back(*it);
		} else if ((*it).dir() == ConnChild) {
			children.push_back(*it);
		}
	}

	setHidden(false);

	// Append header
	lines.push_back("<html>");
	lines.push_back(QString("<h4>%1</h4>").arg(tr("Connections")));

	// Parents.
	if (parents.size() > 0) {
		lines.push_back("<hr/>");
		addLines(lines, parents, QString("↑"));
	}
	
	// Links.
	if (links.size() > 0) {
		lines.push_back("<hr/>");
		addLines(lines, links, QString("←"));
	}
	
	// Children.
	if (children.size() > 0) {
		lines.push_back("<hr/>");
		addLines(lines, children, QString("↓"));
	}

	// Append footer
	lines.push_back("<html>");

	// Update list.
	QString finalDoc = lines.join("\n");
	document()->setPageSize(QSizeF(size().width() - Padding * 2, -1));
	setHtml(finalDoc);

	updateGeometry();
	update();
}

void MarkdownConnectionsWidget::addLines(
	QStringList& lines,
	QList<Connection>& conns,
	QString type
) {
	for (auto it = conns.begin(); it != conns.end(); it++) {
		lines.push_back(
			QString("<p>%1 <a href=\"#%2\">%3</a></p>")
			.arg(type)
			.arg((*it).id())
			.arg((*it).name())
		);
	}
}

