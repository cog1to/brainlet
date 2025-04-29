#include <iostream>

#include <QApplication>
#include <QString>
#include <QIODevice>

#include "model/text_model.h"
#include "widgets/markdown_widget.h"
#include "presenters/text_editor_presenter.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/database_brain_repository.h"

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

	// Make widget.
	MarkdownWidget widget(nullptr, &style);
	widget.resize(600, 600);

	// Make presenter.
	TextEditorPresenter presenter(repo, repo, &widget);

	// Show window.
	widget.show();

	// Load thought.
	presenter.setThought(0);

	// Run app.
	int result = app.exec();
	delete repo;
	return result;
}

