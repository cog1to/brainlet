#ifndef H_SYSTEM_RESOURCE_PROVIDER
#define H_SYSTEM_RESOURCE_PROVIDER

#include <QString>

#include "infra/resource_provider.h"

class SystemResourceProvider: public ResourceProvider {
public:
	SystemResourceProvider();
	QString brainsFolderPath() override;

private:
	QString configFolderPath();
	QString documentsFolderPath();
};

#endif

