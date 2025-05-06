#include <QString>
#include <QStandardPaths>

#include "infra/resource_provider.h"
#include "infra/system_resource_provider.h"

#include <QDebug>

SystemResourceProvider::SystemResourceProvider() {}

QString SystemResourceProvider::brainsFolderPath() {
	QString configPath = documentsFolderPath();
	QString path = QString("%1/brains").arg(configPath);
	qDebug() << "documents path:" << path;
	return path;
}

QString SystemResourceProvider::configFolderPath() {
#if defined(Q_OS_LINUX)
	return QStandardPaths::writableLocation(
		QStandardPaths::AppConfigLocation
	);
#elif defined(Q_OS_DARWIN)
	return QStandardPaths::writableLocation(
		QStandardPaths::AppDataLocation
	);
#endif
}

QString SystemResourceProvider::documentsFolderPath() {
#if defined(Q_OS_LINUX)
	return QStandardPaths::writableLocation(
		QStandardPaths::AppConfigLocation
	);
#elif defined(Q_OS_DARWIN)
	QString docsPath = QStandardPaths::writableLocation(
		QStandardPaths::DocumentsLocation
	);
	return QString("%1/Brainlet").arg(docsPath);
#endif
}
