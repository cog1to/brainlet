#include <QWidget>

#include "infra/dismissable_module.h"
#include "presenters/dismissable_presenter.h"
#include "entity/base_repository.h"

DismissableModule::DismissableModule(
	DismissablePresenter *_presenter,
	QWidget *_widget,
	BaseRepository *_repo
) : presenter(_presenter), widget(_widget), repo(_repo) {}

