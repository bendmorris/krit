#ifndef KRIT_ASSET_ASSETINFO
#define KRIT_ASSET_ASSETINFO

#include "krit/asset/AssetType.h"
#include "krit/math/Dimensions.h"
#include <string>

namespace krit {

struct AssetInfo {
    int id;
    AssetType type;
    std::string path;
    union {
        Dimensions dimensions;
        bool _ = false;
    };
};

}

#endif
