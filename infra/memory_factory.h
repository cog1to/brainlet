#ifndef H_MEMORY_FACTORY
#define H_MEMORY_FACTORY

#include <string>
#include <unordered_map>

#include "widgets/style.h"
#include "infra/dismissable_module.h"
#include "infra/module_factory.h"

class MemoryFactory: public ModuleFactory {
public:
	MemoryFactory(Style *style);
	DismissableModule makeBrainsModule() override;
	DismissableModule makeBrainModule(std::string id) override;

private:
	Style *m_style;
};

#endif
