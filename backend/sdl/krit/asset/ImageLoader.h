#ifndef KRIT_ASSET_IMAGE_LOADER
#define KRIT_ASSET_IMAGE_LOADER

#include "krit/asset/AssetLoader.h"
#include <memory>
#include <string>

namespace krit {

static const std::string IMG_TYPE = "img";

/**
 * Used to load images and initialize GL textures for them.
 */
struct ImageLoader: public AssetLoader {
    ImageLoader() {}

    const std::string &assetType() override { return IMG_TYPE; }
    std::shared_ptr<void> loadAsset(const std::string &id) override;
};

}

#endif
