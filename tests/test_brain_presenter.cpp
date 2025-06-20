#include <iostream>

#include <QApplication>
#include <QString>
#include <QIODevice>

#include "layout/default_layout.h"
#include "widgets/markdown_scroll_widget.h"
#include "widgets/markdown_edit_widget.h"
#include "widgets/canvas_widget.h"
#include "widgets/brain_widget.h"
#include "widgets/thought_details_widget.h"
#include "widgets/container_widget.h"
#include "widgets/markdown_connections_widget.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/canvas_presenter.h"
#include "presenters/brain_presenter.h"
#include "presenters/search_presenter.h"
#include "presenters/connections_presenter.h"
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

	// Make text widget.
	MarkdownEditWidget *markdownWidget = new MarkdownEditWidget(nullptr, &style);
	MarkdownConnectionsWidget *connWidget = new MarkdownConnectionsWidget(nullptr, &style);

	MarkdownScrollWidget *scroll = new MarkdownScrollWidget(nullptr, &style);
	scroll->setMarkdownWidgets(markdownWidget, connWidget);
	scroll->setWidgetResizable(true);

	TextEditorPresenter *markdownPresenter = new TextEditorPresenter(
		&repo,
		&repo,
		scroll
	);

	// Make thought details widget.
	ThoughtDetailsWidget *detailsWidget = new ThoughtDetailsWidget(
		nullptr,
		&style,
		scroll
	);

	// Make canvas widget.
	DefaultLayout *layout = new DefaultLayout(&style);
	CanvasWidget *canvasWidget = new CanvasWidget(
		nullptr,
		&style,
		layout
	);
	CanvasPresenter *canvasPresenter = new CanvasPresenter(
		layout,
		&repo,
		&repo,
		canvasWidget
	);

	// Make canvas container.
	ContainerWidget *containerWidget = new ContainerWidget(
		nullptr,
		&style,
		canvasWidget
	);

	// Make search presenter.
	SearchPresenter *searchPresenter = new SearchPresenter(
		&repo,
		containerWidget->search()
	);

	// History presenter.
	HistoryPresenter *historyPresenter = new HistoryPresenter(
		containerWidget->history()
	);

	// Connections presenter.
	ConnectionsPresenter *connsPresenter = new ConnectionsPresenter(
		connWidget
	);

	// Make brain widget.
	BrainWidget brainWidget = BrainWidget(
		nullptr,
		&style,
		containerWidget,
		detailsWidget
	);
	BrainPresenter brainPresenter(
		&brainWidget,
		canvasPresenter,
		markdownPresenter,
		searchPresenter,
		historyPresenter,
		connsPresenter
	);

	// Show window.
	brainWidget.resize(1200, 600);
	brainWidget.show();

	// Run app.
	return app.exec();
}

