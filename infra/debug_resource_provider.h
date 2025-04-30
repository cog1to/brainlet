#ifndef H_DEBUG_RESOURCE_PROVIDER
#define H_DEBUG_RESOURCE_PROVIDER

#include <QString>

#include "model/thought.h"
#include "infra/resource_provider.h"

class DebugResourceProvider: public ResourceProvider {
public:
	DebugResourceProvider(QString);
	QString brainsFolderPath() override;

private:
	QString m_base;
};

#endif

