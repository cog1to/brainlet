#include <QString>

#include "infra/resource_provider.h"
#include "infra/debug_resource_provider.h"

DebugResourceProvider::DebugResourceProvider(QString path) {
	m_base = path;
}

QString DebugResourceProvider::brainsFolderPath() {
	return m_base;
}

