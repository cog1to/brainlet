#include <iostream>
#include <vector>

#include <QApplication>
#include <QString>
#include <QIODevice>

#include "widgets/tabs_widget.h"
#include "presenters/tabs_presenter.h"
#include "infra/memory_factory.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

	MemoryFactory factory = MemoryFactory(&style);
	TabsWidget widget = TabsWidget(nullptr, &style);
	TabsPresenter presenter = TabsPresenter(&widget, &factory);

	widget.setStyleSheet(
		QString("background-color: %1")
		.arg(style.background().name(QColor::HexArgb))
	);
	widget.resize(900, 600);
	widget.show();

	return app.exec();
}
