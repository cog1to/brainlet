#include <string>
#include <vector>

#include <QObject>

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
	std::string id,
	std::string name
) {
	emit brainSelected(id, name);
}

void BrainListPresenter::onBrainDeleted(std::string id) {
	if (m_repo == nullptr)
		return;
	if (m_widget == nullptr)
		return;

	BrainRepositoryError error = m_repo->deleteBrain(id);
	if (error != BrainRepositoryErrorNone) {
		m_widget->showError(tr("Failed to access file system"));
	} else {
		BrainList list = m_repo->listBrains();
		m_widget->setItems(list);
	}

	emit brainDeleted(id);
}

void BrainListPresenter::onBrainCreated(std::string name) {
	if (m_repo == nullptr)
		return;
	if (m_widget == nullptr)
		return;

	CreateBrainResult result = m_repo->createBrain(name);
	if (result.error == BrainRepositoryErrorNone) {
		BrainList list = m_repo->listBrains();
		m_widget->setItems(list);
	} else if (result.error == BrainRepositoryErrorIO) {
		m_widget->showError(tr("Failed to access file system"));
	} else if (result.error == BrainRepositoryErrorDuplicate) {
		m_widget->showError(tr("A brain with this name already exists"));
	}
}

void BrainListPresenter::onBrainRenamed(
	std::string id,
	std::string name
) {
	if (m_repo == nullptr)
		return;
	if (m_widget == nullptr)
		return;

	BrainRepositoryError result = m_repo->renameBrain(id, name);
	if (result == BrainRepositoryErrorNone) {
		BrainList list = m_repo->listBrains();
		m_widget->setItems(list);
	} else if (result == BrainRepositoryErrorIO) {
		m_widget->showError(tr("Failed to access file system"));
	} else if (result == BrainRepositoryErrorDuplicate) {
		m_widget->showError(tr("A brain with this name already exists"));
	}
}

void BrainListPresenter::onShown() {
	if (m_repo == nullptr)
		return;
	if (m_widget == nullptr)
		return;

	BrainList list = m_repo->listBrains();
	m_widget->setItems(list);
}

void BrainListPresenter::onDismiss() {
	// TODO: Cleanup if needed.
}
