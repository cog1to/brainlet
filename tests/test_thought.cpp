#include <QApplication>
#include <QObject>

#include "widgets/widgets.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	QWidget widget;
	widget.resize(200, 100);

	Style& style = Style::defaultStyle();

	widget.setStyleSheet("background-color: #1b2b34");

	ThoughtWidget thought(&widget, &style, true, "jÀest lorem Ipsum", false, false, true);

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
