#ifndef KRIT_ASSET_ASSETINFO
#define KRIT_ASSET_ASSETINFO

#include "krit/asset/AssetType.h"
#include "krit/math/Dimensions.h"
#include "krit/utils/Panic.h"
#include <string>
#include <unordered_map>

namespace krit {

enum AssetId : int;

union AssetProperties {
    IntDimensions dimensions;
    bool _ = false;

    AssetProperties(int width, int height) : dimensions(width, height) {}
    AssetProperties() : _(false) {}
};

struct AssetInfo {
    AssetId id;
    AssetType type;
    std::string path;
    AssetProperties properties;
};

struct Assets {
    static const AssetInfo &byId(AssetId id) { return _assets[id]; }

    static const AssetInfo &byPath(const std::string &path) {
        auto found = _byPath.find(path);
        if (found == _byPath.end()) {
            panic("unrecognized asset path: %s\n", path.c_str());
        }
        return _assets[found->second];
    }

private:
    static const AssetInfo _assets[];
    static std::unordered_map<std::string, int> _byPath;
};

}

#endif
