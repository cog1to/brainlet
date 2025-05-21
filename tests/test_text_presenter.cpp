#include <iostream>

#include <QApplication>
#include <QString>
#include <QIODevice>

#include "model/text_model.h"
#include "widgets/markdown_edit_widget.h"
#include "widgets/markdown_scroll_widget.h"
#include "presenters/text_editor_presenter.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/memory_repository.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

	// Make repo.
	std::vector<ThoughtEntity> thoughts = {
		ThoughtEntity(0, "Brain"),
		ThoughtEntity(1, "Parent 1"),
		ThoughtEntity(2, "Parent 2"),
		ThoughtEntity(3, "Sibling"),
		ThoughtEntity(4, "Link 1"),
		ThoughtEntity(5, "Link 2")
	};
	std::vector<ConnectionEntity> conns = {
		ConnectionEntity(0, 4, ConnectionType::link),
		ConnectionEntity(0, 5, ConnectionType::link),
		ConnectionEntity(1, 0, ConnectionType::child),
		ConnectionEntity(1, 3, ConnectionType::child),
		ConnectionEntity(1, 4, ConnectionType::child),
		ConnectionEntity(2, 0, ConnectionType::child),
		ConnectionEntity(2, 5, ConnectionType::link),
		ConnectionEntity(3, 2, ConnectionType::link)
	};
	MemoryRepository repo = MemoryRepository(thoughts, conns, 0);

	// Text editor widget and presenter.
	MarkdownEditWidget *markdownWidget = new MarkdownEditWidget(nullptr, &style);

	MarkdownScrollWidget scroll = MarkdownScrollWidget(nullptr, &style);
	scroll.setMarkdownWidget(markdownWidget);
	scroll.setWidgetResizable(true);
	scroll.resize(600, 600);

	// Make presenter.
	TextEditorPresenter presenter(&repo, &repo, &scroll);

	// Show window.
	scroll.show();

	// Load thought.
	presenter.setThought(0);

	// Run app.
	return app.exec();
}

