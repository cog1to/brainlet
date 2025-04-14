#include "infra/dismissable_module.h"
#include "presenters/dismissable_presenter.h"

DismissableModule::DismissableModule(
	DismissablePresenter *_presenter,
	void *_widget,
	void *_repo
) : presenter(_presenter), widget(_widget), repo(_repo) {}

