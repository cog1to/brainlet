#ifndef H_INFRA_FACTORY
#define H_INFRA_FACTORY

#include "infra/dismissable_module.h"

class ModuleFactory {
public:
	virtual DismissableModule makeBrainsModule() = 0;
	virtual DismissableModule makeBrainModule(std::string id) = 0;
};

#endif

