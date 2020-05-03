#ifndef KRIT_ASSET_IMAGE_LOADER
#define KRIT_ASSET_IMAGE_LOADER

#include "krit/asset/AssetLoader.h"
#include "krit/render/ImageData.h"
#include <memory>
#include <string>
#include <vector>

namespace krit {

/**
 * Used to load images and initialize GL textures for them.
 */
struct ImageLoader: public AssetLoader {
    ImageLoader() {}

    AssetType type() override { return ImageAsset; }
    std::shared_ptr<void> loadAsset(const AssetInfo &info) override;

    bool isLoaded(std::shared_ptr<void> img) override {
        return std::static_pointer_cast<ImageData>(img)->texture != 0;
    }
};

}

#endif
