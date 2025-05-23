#include <iostream>

#include <QApplication>
#include <QString>
#include <QIODevice>
#include <QApplication>

#include "layout/default_layout.h"
#include "widgets/canvas_widget.h"
#include "layout/default_layout.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/database_brain_repository.h"
#include "presenters/canvas_presenter.h"

int main(int argc, char *argv[]) {
	Style& style = Style::defaultStyle();
	QApplication app(argc, argv);

	// Make repo.
	QDir dir = QDir("graph_test");
	//if (dir.exists()) {
	//	dir.removeRecursively();
	//}

	DatabaseBrainRepository *repo = DatabaseBrainRepository::fromDir(dir);
	if (repo == nullptr) {
		qDebug("Failed to create a repo");
		return 1;
	}

	// Make layout.
	DefaultLayout *layout = new DefaultLayout(&style);

	// Make widget.
	CanvasWidget widget(nullptr, &style, layout);
	widget.resize(800, 800);

	// Make presenter.
	CanvasPresenter presenter(layout, repo, repo, &widget);

	// Show window.
	widget.show();

	// Run app.
	int result = app.exec();
	delete repo;
	return result;
}

