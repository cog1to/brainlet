#ifndef H_RESOURCE_PROVIDER
#define H_RESOURCE_PROVIDER

#include <string>

#include "model/thought.h"

class ResourceProvider {
public:
	virtual std::string brainsFolderPath() = 0;
};

#endif

