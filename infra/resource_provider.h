#ifndef H_RESOURCE_PROVIDER
#define H_RESOURCE_PROVIDER

#include <QString>

class ResourceProvider {
public:
	virtual QString brainsFolderPath() = 0;
};

#endif

