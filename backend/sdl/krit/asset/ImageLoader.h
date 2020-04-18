#ifndef KRIT_ASSET_IMAGE_LOADER
#define KRIT_ASSET_IMAGE_LOADER

#include "krit/asset/AssetLoader.h"
#include <memory>
#include <string>

using namespace std;
using namespace krit;

namespace krit {

/**
 * Used to load images and initialize GL textures for them.
 */
class ImageLoader: public AssetLoader {
    public:
        ImageLoader() {}

        string assetType() override { return "img"; }
        shared_ptr<void> loadAsset(string id) override;
};

}

#endif
