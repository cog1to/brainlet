#include <vector>

#include <QObject>
#include <QString>

#include "model/brain.h"
#include "entity/brains_repository.h"
#include "widgets/brain_list_widget.h"
#include "presenters/brain_list_presenter.h"

BrainListPresenter::BrainListPresenter(
	BrainListWidget *widget,
	BrainsRepository *repo
) : m_widget(widget), m_repo(repo) {
	connect(
		widget, &BrainListWidget::itemClicked,
		this, &BrainListPresenter::onBrainSelected
	);

	connect(
		widget, &BrainListWidget::itemDeleteClicked,
		this, &BrainListPresenter::onBrainDeleted
	);

	connect(
		widget, &BrainListWidget::newItemCreated,
		this, &BrainListPresenter::onBrainCreated
	);

	connect(
		widget, &BrainListWidget::itemRenamed,
		this, &BrainListPresenter::onBrainRenamed
	);

	connect(
		widget, &BrainListWidget::shown,
		this, &BrainListPresenter::onShown
	);
}

// Slots

void BrainListPresenter::onBrainSelected(
	QString id,
	QString name
) {
	emit brainSelected(id, name);
}

void BrainListPresenter::onBrainDeleted(QString id) {
	if (m_repo == nullptr)
		return;
	if (m_widget == nullptr)
		return;

	BrainRepositoryError error = m_repo->deleteBrain(id);
	if (error != BrainRepositoryErrorNone) {
		m_widget->showError(tr("Failed to access file system"));
		return;
	}

	emit brainDeleted(id);

	reload();
}

void BrainListPresenter::onBrainCreated(QString name) {
	if (m_repo == nullptr)
		return;
	if (m_widget == nullptr)
		return;

	CreateBrainResult result = m_repo->createBrain(name);
	if (result.error == BrainRepositoryErrorIO) {
		m_widget->showError(tr("Failed to access file system"));
		return;
	} else if (result.error == BrainRepositoryErrorDuplicate) {
		m_widget->showError(tr("A brain with this name already exists"));
		return;
	}

	reload();
}

void BrainListPresenter::onBrainRenamed(
	QString id,
	QString name
) {
	if (m_repo == nullptr)
		return;
	if (m_widget == nullptr)
		return;

	BrainRepositoryError result = m_repo->renameBrain(id, name);
	if (result == BrainRepositoryErrorIO) {
		m_widget->showError(tr("Failed to access file system"));
	} else if (result == BrainRepositoryErrorDuplicate) {
		m_widget->showError(tr("A brain with this name already exists"));
	}

	emit brainRenamed(id, name);

	reload();
}

void BrainListPresenter::onShown() {
	if (m_repo == nullptr)
		return;
	if (m_widget == nullptr)
		return;

	reload();
}

void BrainListPresenter::onDismiss() {
	// TODO: Cleanup if needed.
}

// Helpers.

void BrainListPresenter::reload() {
	assert(m_repo != nullptr);
	assert(m_widget != nullptr);

	ListBrainsResult result = m_repo->listBrains();
	if (result.error != BrainRepositoryErrorNone) {
		m_widget->showError(tr("Failed to access file system"));
		return;
	}

	m_widget->setItems(result.list);
}

