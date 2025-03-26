#ifndef H_CONNECTION_LIST_WIDGET
#define H_CONNECTION_LIST_WIDGET

#include <vector>

#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QPushButton>
#include <QEnterEvent>
#include <QPaintEvent>

#include "model/thought.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"

struct ConnectionItem {
public:
	ThoughtId id;
	QString name;
};

// Item widget.

enum ConnectionItemButton {
	ConnButtonLink,
	ConnButtonChild,
	ConnButtonParent,
};

class ConnectionItemWidget: public BaseWidget {
	Q_OBJECT

public:
	ConnectionItemWidget(QWidget*, Style*, bool, ThoughtId, QString);
	// Highlighting.
	void activate();
	void deactivate();

signals:
	void buttonClicked(ConnectionItemWidget*, ThoughtId, QString, ConnectionItemButton);
	void clicked(ConnectionItemWidget*, ThoughtId, QString);
	void hover(ConnectionItemWidget*);

protected:
	void enterEvent(QEnterEvent *) override;
	void leaveEvent(QEvent *) override;
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
	void onLinkClicked();
	void onParentClicked();
	void onChildClicked();

private:
	ThoughtId m_id = InvalidThoughtId;
	QString m_name;
	bool m_showButtons = false;
	bool m_hover = false;
	bool m_pressed = false;
	QHBoxLayout m_layout;
	QPushButton *m_linkButton = nullptr;
	QPushButton *m_parentButton = nullptr;
	QPushButton *m_childButton = nullptr;
	// Helpers.
	QPushButton *makeButton(Style *style, QString title);
};

// List widget.

class ConnectionListWidget: public BaseWidget {
	Q_OBJECT

public:
	ConnectionListWidget(QWidget*, Style*, bool);
	// Updates.
	const std::vector<ConnectionItem> &items();
	void setItems(std::vector<ConnectionItem>);
	int selectedIndex();
	// Size.
	QSize sizeHint() const override;

signals:
	void thoughtSelected(ThoughtId, QString);
	void connectionSelected(ThoughtId, QString, ConnectionType, bool);

public slots:
	void onNextItem();
	void onPrevItem();

private slots:
	void onConnectionSelected(ConnectionItemWidget*, ThoughtId, QString, ConnectionItemButton);
	void onThoughtSelected(ConnectionItemWidget*, ThoughtId, QString);
	void onItemHover(ConnectionItemWidget*);

private:
	QVBoxLayout m_layout;
	bool m_showButtons;
	int m_selectedIdx = -1;
	std::vector<ConnectionItemWidget*> m_widgets;
	std::vector<ConnectionItem> m_items;
	// Helpers.
	QWidget *makeSeparator();
	// Constants.
	static constexpr int MaxItems = 3;
};

#endif
