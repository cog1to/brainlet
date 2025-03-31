#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QPushButton>

#include "widgets/brain_item_widget.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

	QWidget parent = QWidget(nullptr);
	parent.setStyleSheet(
		QString("background-color: %1")
		.arg(style.background().name(QColor::HexRgb))
	);

	BrainItemWidget item = BrainItemWidget(&parent, &style, "id", "My Brain");

	QSize sizeHint = item.sizeHint();
	item.setGeometry(
		20,
		(600-sizeHint.height())/2,
		560,
		sizeHint.height()
	);
	item.show();

	QObject::connect(
		&item, &BrainItemWidget::deleteClicked,
		[&](BrainItemWidget *widget){
			qDebug() << "delete clicked";
		}
	);
	QObject::connect(
		&item, &BrainItemWidget::buttonClicked,
		[&](BrainItemWidget *widget){
			qDebug() << "clicked";
		}
	);

	parent.resize(600, 600);
	parent.show();


	return app.exec();
}
