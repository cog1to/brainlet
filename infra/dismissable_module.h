#ifndef H_INFRA_MODULE
#define H_INFRA_MODULE

#include <vector>

#include "presenters/dismissable_presenter.h"

struct DismissableModule {
public:
	DismissablePresenter *presenter;
	void *widget;
	void *repo;

	DismissableModule(
		DismissablePresenter *presenter,
		void *widget,
		void *repo
	);
};

#endif
