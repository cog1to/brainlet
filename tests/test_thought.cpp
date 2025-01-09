#include <QApplication>
#include <QObject>

#include "widgets/widgets.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	QWidget widget;
	widget.resize(200, 100);

	// Style
	Style style(
		QColor(23, 43, 52, 255),		
		1.0,
		QColor(248, 144, 87, 255),
		2.0,
		QColor(248, 144, 87, 255),
		QFont("Helvetica", 12),
		QColor(228, 83, 75, 255),
		QColor(255, 255, 255, 255)
	);

	widget.setStyleSheet("background-color: #1b2b34");

	ThoughtWidget thought(&widget, &style, "jÀest lorem Ipsum", false, false, true);

	QSize size = thought.sizeHint();
	thought.setGeometry(
		widget.size().width() / 2.0 - size.width() / 2.0,
		widget.size().height() / 2.0 - size.height() / 2.0,
		size.width(),
		size.height()
	);

	widget.show();
	return app.exec();
}
