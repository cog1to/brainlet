#include <QApplication>
#include <QString>
#include <QStyleFactory>
#include <QIcon>

#include "widgets/tabs_widget.h"
#include "presenters/tabs_presenter.h"
#include "infra/database_module_factory.h"
#include "infra/system_resource_provider.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);
	app.setApplicationName("Brainlet");
	app.setApplicationDisplayName("Brainlet");
	app.setWindowIcon(QIcon(":/icons/app.png"));

	Style& style = Style::defaultStyle();

	SystemResourceProvider provider = SystemResourceProvider();
	DatabaseModuleFactory factory = DatabaseModuleFactory(&style, &provider);
	TabsWidget *widget = new TabsWidget(nullptr, &style);
	TabsPresenter *presenter = new TabsPresenter(widget, &factory);

	widget->resize(1024, 760);
	widget->show();

	return app.exec();
}

