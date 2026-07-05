#ifndef H_ASSETS_REPOSITORY
#define H_ASSETS_REPOSITORY

#include <QString>

enum AssetsRepositoryError {
	AssetsRepositoryErrorNone,
	AssetsRepositoryErrorIO
};

struct AssetSaveResult {
	AssetsRepositoryError error;
	QString assetPath;
public:
	AssetSaveResult(
		AssetsRepositoryError _err,
		QString _assetPath
	): error(_err), assetPath(_assetPath) {};
};

class AssetsRepository {
public:
	virtual AssetSaveResult saveAsset(QString &filePath) = 0;
};

#endif
