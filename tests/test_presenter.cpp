#include <QApplication>

#include "widgets/canvas_widget.h"
#include "layout/default_layout.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/memory_repository.h"
#include "presenters/canvas_presenter.h"

int main(int argc, char *argv[]) {
	Style& style = Style::defaultStyle();
	QApplication app(argc, argv);

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

	// Make layout.
	DefaultLayout *layout = new DefaultLayout(&style);

	// Make widget.
	CanvasWidget widget(nullptr, &style, layout);
	widget.resize(800, 800);

	// Make presenter.
	CanvasPresenter presenter(layout, &repo, &repo, &widget);

	// Show window.
	widget.show();

	// Run app.
	return app.exec();
}

