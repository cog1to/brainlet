#ifndef H_DEBUG_RESOURCE_PROVIDER
#define H_DEBUG_RESOURCE_PROVIDER

#include "model/thought.h"
#include "infra/resource_provider.h"

class DebugResourceProvider: public ResourceProvider {
public:
	DebugResourceProvider(std::string);
	std::string brainsFolderPath() override;

private:
	std::string m_base;
};

#endif

