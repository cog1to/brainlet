#include <QString>

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
#include "widgets/markdown_scroll_widget.h"
#include "widgets/markdown_edit_widget.h"
#include "widgets/canvas_widget.h"
#include "widgets/brain_widget.h"
#include "widgets/thought_details_widget.h"
#include "widgets/container_widget.h"
#include "presenters/brain_list_presenter.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/canvas_presenter.h"
#include "presenters/brain_presenter.h"
#include "presenters/search_presenter.h"
#include "infra/database_module_factory.h"

DatabaseModuleFactory::DatabaseModuleFactory(
	Style *style,
	ResourceProvider *provider
) : m_style(style), m_provider(provider) {}

DismissableModule DatabaseModuleFactory::makeBrainsModule() {
	QString path = m_provider->brainsFolderPath();
	FolderBrainsRepository *repo = new FolderBrainsRepository(path);

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

DismissableModule DatabaseModuleFactory::makeBrainModule(QString id) {
	// Get or create a repo.
	QString path = QString("%1/%2")
		.arg(m_provider->brainsFolderPath())
		.arg(id);
	QDir dir = QDir(path);
	DatabaseBrainRepository *repo = DatabaseBrainRepository::fromDir(dir);

	// Text editor widget and presenter.
	MarkdownEditWidget *markdownWidget = new MarkdownEditWidget(nullptr, m_style);

	MarkdownScrollWidget *scroll = new MarkdownScrollWidget(nullptr, m_style);
	scroll->setMarkdownWidget(markdownWidget);
	scroll->setWidgetResizable(true);

	TextEditorPresenter *markdownPresenter = new TextEditorPresenter(
		repo,
		repo,
		scroll
	);

	// Thought details widget for editing individual thoughts.
	ThoughtDetailsWidget *detailsWidget = new ThoughtDetailsWidget(
		nullptr,
		m_style,
		scroll
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

