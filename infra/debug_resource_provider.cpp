#include <string>
#include <cstdio>

#include "infra/resource_provider.h"
#include "infra/debug_resource_provider.h"

DebugResourceProvider::DebugResourceProvider(std::string path) {
	m_base = path;
}

std::string DebugResourceProvider::brainsFolderPath() {
	return m_base;
}

