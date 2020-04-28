#ifndef KRIT_ASSET_IMAGE_LOADER
#define KRIT_ASSET_IMAGE_LOADER

#include "krit/asset/AssetLoader.h"
#include "krit/render/ImageData.h"
#include <memory>
#include <string>
#include <vector>

namespace krit {

using PreloadedImages = std::vector<std::shared_ptr<ImageData>>;

/**
 * Used to load images and initialize GL textures for them.
 */
struct ImageLoader: public AssetLoader {
    ImageLoader() {}

    AssetType type() override { return ImageAsset; }
    std::shared_ptr<void> loadAsset(const AssetInfo &info) override;

    bool ready(std::shared_ptr<ImageData> img) { return img->texture != 0; }
    bool imagesAreReady(PreloadedImages &preload) {
        int readyCount = 0;
        for (auto img : preload) {
            if (!ready(img)) {
                return false;
            }
        }
        return true;
    }
};

}

#endif
