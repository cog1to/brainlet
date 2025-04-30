#ifndef H_INFRA_FACTORY
#define H_INFRA_FACTORY

#include <QString>

#include "infra/dismissable_module.h"

class ModuleFactory {
public:
	virtual DismissableModule makeBrainsModule() = 0;
	virtual DismissableModule makeBrainModule(QString id) = 0;
};

#endif

