#ifndef H_INFRA_MODULE
#define H_INFRA_MODULE

#include <QWidget>

#include "presenters/dismissable_presenter.h"
#include "entity/base_repository.h"

struct DismissableModule {
public:
	DismissablePresenter *presenter;
	QWidget *widget;
	BaseRepository *repo;

	DismissableModule(
		DismissablePresenter *presenter,
		QWidget *widget,
		BaseRepository *repo
	);
};

#endif
