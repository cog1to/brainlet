#ifndef H_RESOURCE_PROVIDER
#define H_RESOURCE_PROVIDER

#include <QString>

#include "model/thought.h"

class ResourceProvider {
public:
	virtual QString brainsFolderPath() = 0;
};

#endif

