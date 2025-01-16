#include <QApplication>
#include <QObject>

#include "widgets/widgets.h"

int main(int argc, char *argv[]) {
	int normalSize = 300;
	int maxSize = 700;

	QApplication app(argc, argv);
	QWidget widget;
	widget.resize(800, 800);

	Style& style = Style::defaultStyle();

	widget.setStyleSheet("background-color: #1b2b34");

	ThoughtWidget thought(&widget, &style, 0, true, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent in egestas diam, in molestie quam. Vivamus non dolor laoreet velit mattis accumsan. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Quisque eu scelerisque mauris. In hac habitasse platea dictumst.", false, false, true);

	QSize size = thought.sizeHint();
	int actualWidth = std::min(normalSize, size.width());
	thought.setGeometry(
		(widget.size().width() - actualWidth) / 2.0,
		(widget.size().height() - size.height()) / 2.0,
		actualWidth,
		size.height()
	);

	QObject::connect(&thought, &ThoughtWidget::activated, [&](){
		QSize fullSize = thought.sizeForWidth(maxSize);
		thought.setGeometry(
			(widget.size().width() - fullSize.width()) / 2.0,
			(widget.size().height() - fullSize.height()) / 2.0,
			fullSize.width(),
			fullSize.height()
		);
	});

	QObject::connect(&thought, &ThoughtWidget::deactivated, [&](){
		QSize size = thought.sizeHint();
		int actualWidth = std::min(normalSize, size.width());
		thought.setGeometry(
			(widget.size().width() - actualWidth) / 2.0,
			(widget.size().height() - size.height()) / 2.0,
			actualWidth,
			size.height()
		);
	});

	widget.show();
	return app.exec();
}

