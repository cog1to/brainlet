#ifndef H_MEMORY_FACTORY
#define H_MEMORY_FACTORY

#include <unordered_map>

#include <QString>

#include "widgets/style.h"
#include "infra/dismissable_module.h"
#include "infra/module_factory.h"
#include "entity/memory_repository.h"

class MemoryFactory: public ModuleFactory {
public:
	MemoryFactory(Style *style);
	DismissableModule makeBrainsModule() override;
	DismissableModule makeBrainModule(QString id) override;

private:
	Style *m_style;
	MemoryRepository *m_list_repo;
};

#endif
