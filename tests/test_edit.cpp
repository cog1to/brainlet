#include <QApplication>
#include <QObject>
#include <QColor>

#include "widgets/widgets.h"

void resizeMax(ThoughtWidget *thought, int maxSize) {
	QSize fullSize = thought->sizeForWidth(maxSize);
	thought->setGeometry(
		(thought->parentWidget()->size().width() - fullSize.width()) / 2.0,
		(thought->parentWidget()->size().height() - fullSize.height()) / 2.0,
		fullSize.width(),
		fullSize.height()
	);
}

void resizeMin(ThoughtWidget *thought, int minSize) {
	QSize size = thought->sizeHint();
	int actualWidth = std::min(minSize, size.width());
	thought->setGeometry(
		(thought->parentWidget()->size().width() - actualWidth) / 2.0,
		(thought->parentWidget()->size().height() - size.height()) / 2.0,
		actualWidth,
		size.height()
	);
}

int main(int argc, char *argv[]) {
	const int normalSize = 300;
	const int maxSize = 700;

	QApplication app(argc, argv);
	QWidget widget;
	widget.resize(800, 800);

	Style& style = Style::defaultStyle();

	widget.setStyleSheet(
		QString("background-color: %1").arg(
			style.background().name(QColor::HexRgb)
		)
	);

	ThoughtWidget thought(
		&widget,
		&style,
		0,
		false,
		"Lorem ipsum. askdlhsa asd asd s aw r34qw",
		false, false, true
	);

	// Initial geometry.
	resizeMin(&thought, normalSize);

	// Active state geometry.
	QObject::connect(&thought, &ThoughtWidget::activated, [&](){
		resizeMax(&thought, maxSize);
	});

	// Non-active state geometry.
	QObject::connect(&thought, &ThoughtWidget::deactivated, [&](){
		resizeMin(&thought, normalSize);
	});

	// Text change geometry.
	QObject::connect(&thought, &ThoughtWidget::textChanged, [&](){
		resizeMax(&thought, maxSize);
	});

	widget.show();
	return app.exec();
}

