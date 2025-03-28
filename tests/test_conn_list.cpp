#include <QApplication>
#include <QWidget>
#include <QString>
#include <QFrame>
#include <QDebug>

#include "model/thought.h"
#include "widgets/style.h"
#include "widgets/connection_list_widget.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	QWidget widget;
	widget.resize(600, 400);

	Style& style = Style::defaultStyle();
	widget.setStyleSheet(
		QString("background-color: %1;")
			.arg(style.background().name(QColor::HexRgb))
	);

	// Add connections.
	std::vector<ConnectionItem> items = {
		ConnectionItem{
			.id = 1,
			.name = "Thought One Long Name 123\%\%$%^",
		},
		ConnectionItem{
			.id = 3,
			.name = "Lorem Ipsum",
		},
		ConnectionItem{
			.id = 4,
			.name = "42",
		},
		ConnectionItem{
			.id = 5,
			.name = "Thought One",
		},
	};

	// Create connection list widget.
	ConnectionListWidget listWidget(&widget, &style, true);
	listWidget.setItems(items);

	QFrame frame = QFrame(&widget);
	ConnectionListWidget noButtonsWidget(&frame, &style, false);
	noButtonsWidget.setItems(items);

	QSize buttonsSize = listWidget.sizeHint();
	QSize noButtonsSize = noButtonsWidget.sizeHint();
	QSize parentSize = widget.geometry().size();

	listWidget.setGeometry(
		(parentSize.width() - buttonsSize.width()) / 2,
		(parentSize.height() / 4) - (buttonsSize.height() / 2),
		buttonsSize.width(),
		buttonsSize.height()
	);

	frame.setGeometry(
		(parentSize.width() - noButtonsSize.width()) / 2,
		(parentSize.height() * 3 / 4) - (noButtonsSize.height() / 2),
		noButtonsSize.width(),
		noButtonsSize.height()
	);
	noButtonsWidget.setGeometry(frame.rect());

	// Connect
	QObject::connect(
		&listWidget,
		&ConnectionListWidget::thoughtSelected,
		[&](ThoughtId id, QString name){
			qDebug() << "thought selected from first list:" << id;
		}	
	);
	QObject::connect(
		&listWidget,
		&ConnectionListWidget::connectionSelected,
		[&](ThoughtId id, QString name, ConnectionType type, bool incoming){
			qDebug() << "connection selected from first list:" << id << type << incoming;
		}	
	);
	QObject::connect(
		&noButtonsWidget,
		&ConnectionListWidget::thoughtSelected,
		[&](ThoughtId id, QString name){
			qDebug() << "thought selected from second list:" << id;
		}	
	);
	QObject::connect(
		&noButtonsWidget,
		&ConnectionListWidget::connectionSelected,
		[&](ThoughtId id, QString name, ConnectionType type, bool incoming){
			qDebug() << "connection selected from second list:" << id << type << incoming;
		}	
	);

	// Show window.
	widget.show();

	// Run app.
	return app.exec();
}

