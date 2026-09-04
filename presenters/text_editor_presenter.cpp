#include <QString>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDebug>

#include "model/model.h"
#include "presenters/text_editor_presenter.h"
#include "presenters/search_presenter.h"
#include "widgets/search_widget.h"

TextEditorPresenter::TextEditorPresenter(
	TextRepository *repo,
	SearchRepository *search,
	AssetsRepository *assets,
	GraphRepository *graph,
	MarkdownScrollWidget *view,
	ThoughtDetailsWidget *detailsView
)
	: m_repository(repo),
	m_searchRepository(search),
	m_assetsRepository(assets),
	m_graphRepository(graph),
	m_view(view),
	m_detailsView(detailsView)
{
	m_editView = view->markdownWidget();
	m_editView->setImageAssetProvider(this);

	connect(
		m_editView, SIGNAL(textChanged(QString&)),
		this, SLOT(onTextChanged(QString&))
	);

	connect(
		this, SIGNAL(textError(MarkdownScrollError)),
		view, SLOT(onError(MarkdownScrollError))
	);

	connect(
		m_editView, SIGNAL(nodeLinkSelected(ThoughtId)),
		this, SIGNAL(nodeLinkSelected(ThoughtId))
	);

	connect(
		m_editView, SIGNAL(assetLinkSelected(QString)),
		this, SLOT(onAssetLinkSelected(QString))
	);

	connect(
		m_editView, SIGNAL(urlLinkSelected(QString)),
		this, SLOT(onUrlSelected(QString))
	);

	connect(
		m_editView, SIGNAL(nodeInsertionActivated(QPoint)),
		this, SLOT(onNodeInsertion(QPoint))
	);

	connect(
		m_editView, SIGNAL(imageInsertionActivated()),
		this, SLOT(onImageInsertion())
	);

	connect(
		m_detailsView, SIGNAL(titleEditConfirmed(QString, std::function<void(bool)>)),
		this, SLOT(onTitleChanged(QString, std::function<void(bool)>))
	);
}

// Loading.

void TextEditorPresenter::setThought(ThoughtId id) {
	if (m_view == nullptr)
		return;

	// Force save.
	if (m_editView->isDirty()) {
		QString text = m_editView->text();
		onTextChanged(text);
	}

	// Set ID.
	m_id = id;

	// Empty state.
	if (m_id == InvalidThoughtId) {
		QString empty = QString();
		m_editView->load(empty);
		return;
	}

	if (m_repository == nullptr)
		return;

	// Valid state.
	GetResult result = m_repository->getText(m_id);
	if (result.error != TextRepositoryError::TextRepositoryErrorNone) {
		m_id = InvalidThoughtId;
		emit textError(MarkdownScrollError::MarkdownScrollIOError);
		return;
	}

	QString text = result.result;
	qDebug() << "loaded" << text;
	m_editView->load(text);
}

// Events.

void TextEditorPresenter::onTextChanged(QString& text) {
	if (m_repository == nullptr)
		return;
	if (m_id == InvalidThoughtId)
		return;

	SaveResult result = m_repository->saveText(m_id, text);

	if (result.error != TextRepositoryError::TextRepositoryErrorNone) {
		emit textError(MarkdownScrollError::MarkdownScrollIOError);
	}
}

void TextEditorPresenter::onNodeInsertion(QPoint point) {
	if (m_searchRepository == nullptr)
		return;
	if (m_editView == nullptr)
		return;

	SearchWidget *widget = new SearchWidget(
		nullptr,
		m_editView->style(),
		true,
		tr("Connect to..."),
		true
	);
	SearchPresenter *presenter = new SearchPresenter(
		m_searchRepository,
		widget
	);
	m_search = presenter;

	connect(
		presenter, SIGNAL(searchCanceled()),
		this, SLOT(onSearchCanceled())
	);
	connect(
		presenter, &SearchPresenter::connectionSelected,
		this, &TextEditorPresenter::onConnectionSelected
	);
	connect(
		presenter, SIGNAL(searchItemSelected(ThoughtId, QString)),
		this, SLOT(onThoughtSelected(ThoughtId, QString))
	);

	m_editView->showSearchWidget(widget, point);
}

void TextEditorPresenter::onImageInsertion() {
	QFileDialog dialog(m_view);

	QStringList locations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
	if (locations.size() > 0) {
		dialog.setDirectory(locations[0]);
	}

	dialog.setNameFilter(tr("Images (*.png *.apng *.jpg *.jpeg *.bmp *.gif *.tiff *.webp *.tif *.jfif *.jfi)"));

	if (dialog.exec()) {
		QStringList fileNames = dialog.selectedFiles();
		if (fileNames.size() == 0) {
			return;
		}

		QString &fileName = fileNames[0];
		// Copy file to assets and signal the editor.
		AssetSaveResult result = m_assetsRepository->saveAsset(fileName);

		if (result.error == AssetsRepositoryErrorNone) {
			m_editView->insertAssetLink(result.assetPath);
			m_editView->setFocus();
		} else {
			m_view->onError(MarkdownScrollAssetIOError);
		}
	}
}

void TextEditorPresenter::onSearchCanceled() {
	if (m_search != nullptr) {
		delete m_search;
		m_search = nullptr;
	}

	if (m_editView != nullptr) {
		m_editView->hideSearchWidget();
	}
}

void TextEditorPresenter::onConnectionSelected(
	ThoughtId id,
	QString name,
	ConnectionType type,
	bool incoming
) {
	bool result = false;

	if (m_repository == nullptr)
		return;

	if (incoming) {
		result = m_repository->connectThoughts(id, m_id, type);
	} else {
		result = m_repository->connectThoughts(m_id, id, type);
	}

	if (result) {
		m_editView->hideSearchWidget();
		m_editView->insertNodeLink(id, name);
		m_editView->setFocus();
		emit connectionCreated();
	} else {
		m_view->onError(MarkdownScrollIOError);
	}
}

void TextEditorPresenter::onThoughtSelected(
	ThoughtId id,
	QString name
) {
	if (m_editView == nullptr)
		return;

	m_editView->hideSearchWidget();
	m_editView->insertNodeLink(id, name);
	m_editView->setFocus();
}

void TextEditorPresenter::onDismiss() {
	if (m_editView == nullptr)
		return;

	if (m_editView->isDirty()) {
		QString text = m_editView->text();
		onTextChanged(text);
	}
}

void TextEditorPresenter::onAssetLinkSelected(QString assetName) {
	QString filePath = m_assetsRepository->getAssetPath(assetName);
	if (filePath.isNull()) {
		return;
	}

	QFileInfo info = QFileInfo(filePath);
	QString path = QString("file://%1").arg(info.absolutePath());
	QDesktopServices::openUrl(QUrl(path));
}

void TextEditorPresenter::onUrlSelected(QString path) {
	QDesktopServices::openUrl(QUrl(path));
}

void TextEditorPresenter::onTitleChanged(
	QString text,
	std::function<void(bool)> callback
) {
	if (m_graphRepository == nullptr)
		return;

	if (m_id == InvalidThoughtId)
		return;

	std::string value = text.toStdString();
	bool result = m_graphRepository->updateThought(m_id, value);
	callback(result);

	if (result) {
		emit thoughtRenamed(m_id, text);
	}
}

// Image asset provider.

QPixmap *TextEditorPresenter::pixmapForAsset(QString& assetName) {
	if (m_assetsRepository == nullptr) {
		return nullptr;
	}

	QString filePath = m_assetsRepository->getAssetPath(assetName);
	if (filePath.isNull()) {
		return nullptr;
	}

	QPixmap *pixmap = new QPixmap(filePath);
	if (pixmap->isNull()) {
		delete pixmap;
		return nullptr;
	}

	return pixmap;
}
