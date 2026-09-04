#ifndef H_TEXT_EDITOR_PRESENTER
#define H_TEXT_EDITOR_PRESENTER

#include <QObject>
#include <QString>
#include <QPixmap>

#include "model/thought.h"
#include "entity/text_repository.h"
#include "entity/search_repository.h"
#include "entity/graph_repository.h"
#include "entity/assets_repository.h"
#include "widgets/markdown_edit_widget.h"
#include "widgets/markdown_scroll_widget.h"
#include "widgets/thought_details_widget.h"
#include "presenters/search_presenter.h"
#include "presenters/dismissable_presenter.h"
#include "common/image_asset_provider.h"

class TextEditorPresenter: public DismissablePresenter, public ImageAssetProvider {
	Q_OBJECT

public:
	TextEditorPresenter(
		TextRepository*,
		SearchRepository*,
		AssetsRepository*,
		GraphRepository*,
		MarkdownScrollWidget*,
		ThoughtDetailsWidget*
	);
	void setThought(ThoughtId);
	QPixmap *pixmapForAsset(QString& assetName) override;

signals:
	void textError(MarkdownScrollError);
	void nodeLinkSelected(ThoughtId);
	void connectionCreated();
	void thoughtRenamed(ThoughtId, QString);

public slots:
	void onDismiss() override;

private slots:
	void onTextChanged(QString&);
	void onNodeInsertion(QPoint);
	void onImageInsertion();
	void onSearchCanceled();
	void onConnectionSelected(ThoughtId, QString, ConnectionType, bool);
	void onThoughtSelected(ThoughtId, QString);
	void onAssetLinkSelected(QString);
	void onUrlSelected(QString);
	void onTitleChanged(QString text, std::function<void(bool)> callback);

private:
	ThoughtId m_id = InvalidThoughtId;
	// Dependencies.
	GraphRepository *m_graphRepository = nullptr;
	TextRepository *m_repository = nullptr;
	MarkdownScrollWidget *m_view = nullptr;
	MarkdownEditWidget *m_editView = nullptr;
	ThoughtDetailsWidget *m_detailsView = nullptr;
	// Search.
	SearchRepository *m_searchRepository = nullptr;
	SearchPresenter *m_search = nullptr;
	// Assets.
	AssetsRepository *m_assetsRepository = nullptr;
};

#endif

