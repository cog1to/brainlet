#include <QApplication>
#include <QObject>

#include "widgets/anchor_widget.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	QWidget widget;
	widget.resize(200, 100);

	Style& style = Style::defaultStyle();

	widget.setStyleSheet("background-color: #1b2b34");

	AnchorWidget activeAnchor(&widget, &style, AnchorType::AnchorChild, true);
	AnchorWidget nonActiveAnchor(&widget, &style, AnchorType::AnchorLink, false);

	QSize size = activeAnchor.sizeHint();
	activeAnchor.move(
		widget.size().width() / 3.0 - size.width() / 2.0,
		widget.size().height() / 2.0 - size.height() / 2.0
	);
	nonActiveAnchor.move(
		widget.size().width() * 2.0 / 3.0 - size.width() / 2.0,
		widget.size().height() / 2.0 - size.height() / 2.0
	);

	widget.show();
	return app.exec();
}
