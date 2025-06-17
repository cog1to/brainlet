#include <iostream>

#include <QApplication>
#include <QString>
#include <QIODevice>

#include "model/new_text_model.h"
#include "widgets/markdown_scroll_widget.h"
#include "widgets/markdown_edit_widget.h"
#include "presenters/text_editor_presenter.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/database_brain_repository.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

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
	MarkdownEditWidget *markdownWidget = new MarkdownEditWidget(nullptr, &style);

	MarkdownScrollWidget scroll = MarkdownScrollWidget(nullptr, &style);
	scroll.setMarkdownWidgets(markdownWidget, nullptr);
	scroll.setWidgetResizable(true);
	scroll.resize(600, 600);

	// Make presenter.
	TextEditorPresenter presenter(repo, repo, &scroll);

	// Show window.
	scroll.show();

	// Load thought.
	presenter.setThought(0);

	// Run app.
	int result = app.exec();
	delete repo;
	return result;
}

