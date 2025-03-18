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

signals:
	void buttonClicked(ConnectionItemWidget*, ThoughtId, ConnectionItemButton);
	void clicked(ConnectionItemWidget*, ThoughtId);

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

signals:
	void thoughtSelected(ThoughtId);
	void connectionSelected(ThoughtId, ConnectionType, bool);

private slots:
	void onConnectionSelected(ConnectionItemWidget*, ThoughtId, ConnectionItemButton);
	void onThoughtSelected(ConnectionItemWidget*, ThoughtId);

private:
	QVBoxLayout m_layout;
	bool m_showButtons;
	std::vector<ConnectionItem> m_items;
	// Helpers.
	QWidget *makeSeparator();
	// Constants.
	static constexpr int MaxItems = 3;
};

#endif
