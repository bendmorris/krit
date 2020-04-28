#ifndef KRIT_ASSET_ASSETINFO
#define KRIT_ASSET_ASSETINFO

#include "krit/asset/AssetType.h"
#include "krit/math/Dimensions.h"
#include <string>

namespace krit {

union AssetProperties {
    IntDimensions dimensions;
    bool _ = false;

    AssetProperties(int width, int height): dimensions(width, height) {}
    AssetProperties(): _(false) {}
};

struct AssetInfo {
    int id;
    AssetType type;
    std::string path;
    AssetProperties properties;
};

}

#endif
