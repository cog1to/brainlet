#include <string>

#include "infra/dismissable_module.h"
#include "infra/module_factory.h"
#include "infra/memory_factory.h"
#include "model/thought.h"
#include "model/text_model.h"
#include "layout/default_layout.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/memory_repository.h"
#include "widgets/brain_list_widget.h"
#include "widgets/markdown_widget.h"
#include "widgets/canvas_widget.h"
#include "widgets/brain_widget.h"
#include "widgets/thought_details_widget.h"
#include "widgets/container_widget.h"
#include "presenters/brain_list_presenter.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/canvas_presenter.h"
#include "presenters/brain_presenter.h"
#include "presenters/search_presenter.h"

MemoryFactory::MemoryFactory(Style *style) {
	m_style = style;
}

DismissableModule MemoryFactory::makeBrainsModule() {
	std::vector<ThoughtEntity> thoughts;
	std::vector<ConnectionEntity> conns;

	MemoryRepository *repo = new MemoryRepository(thoughts, conns, 0);
	BrainListWidget *widget = new BrainListWidget(nullptr, m_style);
	BrainListPresenter *presenter = new BrainListPresenter(widget, repo);

	widget->setStyleSheet(
		QString("background-color: %1")
		.arg(m_style->background().name(QColor::HexArgb))
	);

	return DismissableModule(
		presenter,
		widget,
		repo		
	);
}

DismissableModule MemoryFactory::makeBrainModule(std::string id) {
	// TODO: Needs a mechanism to destroy every sub-presenter and
	// sub-widget.
	//
	// 1. Dispose bag pattern as part of the module/widget.
	// 2. Ownership of sub-components in every component.
	// 3. ?

	// Get or create a repo.
	std::vector<ThoughtEntity> thoughts;
	std::vector<ConnectionEntity> conns;
	MemoryRepository *repo = new MemoryRepository(thoughts, conns, 0);

	// Text editor widget and presenter.
	MarkdownWidget *markdownWidget = new MarkdownWidget(nullptr, m_style);
	TextEditorPresenter *markdownPresenter = new TextEditorPresenter(
		repo,
		repo,
		markdownWidget
	);

	// Thought details widget for editing individual thoughts.
	ThoughtDetailsWidget *detailsWidget = new ThoughtDetailsWidget(
		nullptr,
		m_style,
		markdownWidget
	);

	// Canvas widget to draw connection graph.
	DefaultLayout *layout = new DefaultLayout(m_style);
	CanvasWidget *canvasWidget = new CanvasWidget(nullptr, m_style, layout);
	CanvasPresenter *canvasPresenter = new CanvasPresenter(
		layout,
		repo,
		repo,
		canvasWidget
	);

	// Canvas container with additional controls.
	ContainerWidget *containerWidget = new ContainerWidget(
		nullptr,
		m_style,
		canvasWidget
	);

	// Search bar presenter.
	SearchPresenter *searchPresenter = new SearchPresenter(
		repo,
		containerWidget->search()
	);

	// Top-level brain widget.
	BrainWidget *brainWidget = new BrainWidget(
		nullptr, m_style,
		containerWidget,
		detailsWidget
	);

	// Top-level presenter.
	BrainPresenter *brainPresenter = new BrainPresenter(
		brainWidget,
		canvasPresenter,
		markdownPresenter,
		searchPresenter
	);

	return DismissableModule(
		brainPresenter,
		brainWidget,
		repo		
	);
}

