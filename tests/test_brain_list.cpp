#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "model/model.h"
#include "widgets/brain_item_widget.h"
#include "widgets/brain_list_widget.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

	QWidget parent = QWidget(nullptr);
	parent.resize(600, 600);
	parent.setStyleSheet(
		QString("background-color: %1")
		.arg(style.background().name(QColor::HexRgb))
	);

	std::vector<Brain> items = {
		Brain("b1", "My Brain", 1),
		Brain("b2", "Brain with long name", 2),
		Brain("b3", "Brain with very long name lorem ipsum dolor sit amet", 3)
	};
	
	BrainListWidget listWidget(nullptr, &style);
	BrainList list = BrainList(items, 100, "memory");
	listWidget.setItems(list);

	QVBoxLayout layout(&parent);
	layout.addWidget(&listWidget);
	parent.show();

	return app.exec();
}
