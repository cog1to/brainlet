#ifndef H_HISTORY_WIDGET
#define H_HISTORY_WIDGET

#include <QFrame>
#include <QSize>
#include <QString>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QList>
#include <QHBoxLayout>
#include <QResizeEvent>

#include "model/thought.h"
#include "widgets/style.h"

class HistoryItem: public QFrame {
	Q_OBJECT

public:
	HistoryItem(QWidget*, Style*, ThoughtId, QString&);
	QSize sizeHint() const override;
	QString& name();
	ThoughtId id();

signals:
	void clicked(HistoryItem*);

protected:
	void paintEvent(QPaintEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;

private:
	Style *m_style;
	QString m_name;
	ThoughtId m_id;
};

class HistoryWidget: public QFrame {
	Q_OBJECT

public:
	HistoryWidget(QWidget *parent, Style *style);
	~HistoryWidget();
	QSize sizeHint() const override;
	// Update.
	void addItem(ThoughtId, QString&);

signals:
	void itemSelected(ThoughtId, QString&);

protected:
	void resizeEvent(QResizeEvent*) override;

private slots:
	void onItemClicked(HistoryItem*);

private:
	Style *m_style;
	QList<HistoryItem *> m_items;
	int m_prevVisibleCount = 0;
	// Internal.
	void relayout();
};

#endif

