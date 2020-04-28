#ifndef KRIT_ASSET_ASSET_LOADER
#define KRIT_ASSET_ASSET_LOADER

#include <memory>
#include <string>
#include "krit/asset/AssetInfo.h"
#include "krit/asset/AssetType.h"

namespace krit {

struct AssetLoader {
    virtual AssetType type() { return TextAsset; }
    virtual size_t initialSize() { return 16; }
    virtual std::shared_ptr<void> loadAsset(const AssetInfo &info) = 0;
};

}

#endif
