#ifndef H_DATABASE_MODULE_FACTORY
#define H_DATABASE_MODULE_FACTORY

#include <unordered_map>

#include <QString>

#include "widgets/style.h"
#include "infra/dismissable_module.h"
#include "infra/module_factory.h"
#include "infra/resource_provider.h"
#include "entity/database_brain_repository.h"
#include "entity/folder_brains_repository.h"

class DatabaseModuleFactory: public ModuleFactory {
public:
	DatabaseModuleFactory(Style*, ResourceProvider*);
	DismissableModule makeBrainsModule() override;
	DismissableModule makeBrainModule(QString) override;

private:
	Style *m_style;
	ResourceProvider *m_provider;
};

#endif
