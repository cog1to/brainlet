#ifndef H_MARKDOWN_CONNECTIONS_WIDGET
#define H_MARKDOWN_CONNECTIONS_WIDGET

#include <QWidget>
#include <QTextBrowser>
#include <QList>

#include "widgets/style.h"
#include "model/connection.h"

class MarkdownConnectionsWidget: public QTextBrowser {
	Q_OBJECT

public:
	MarkdownConnectionsWidget(QWidget*, Style*);
	QSize sizeHint() const override;
	void setConnections(QList<Connection>);

private:
	Style *m_style;
	// Helpers.
	void addLines(QStringList&, QList<Connection>&, QString);
	// Constants
	static constexpr int Padding = 8;
};

#endif
