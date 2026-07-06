#ifndef H_IMAGE_ASSET_PROVIDER
#define H_IMAGE_ASSET_PROVIDER

#include <QPixmap>
#include <QString>

class ImageAssetProvider {
public:
	virtual QPixmap *pixmapForAsset(QString& assetName) = 0;
};

#endif
