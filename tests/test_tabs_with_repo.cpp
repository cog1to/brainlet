#include <iostream>
#include <vector>

#include <QApplication>
#include <QString>
#include <QStyleFactory>

#include "widgets/tabs_widget.h"
#include "presenters/tabs_presenter.h"
#include "infra/database_module_factory.h"
#include "infra/debug_resource_provider.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

	DebugResourceProvider provider = DebugResourceProvider("brains");
	DatabaseModuleFactory factory = DatabaseModuleFactory(&style, &provider);
	TabsWidget widget = TabsWidget(nullptr, &style);
	TabsPresenter presenter = TabsPresenter(&widget, &factory);

	widget.resize(900, 600);
	widget.show();

	return app.exec();
}
