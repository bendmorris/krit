#ifndef KRIT_ASSET_ASSETS
#define KRIT_ASSET_ASSETS

#include "krit/asset/AssetType.h"
#include "krit/math/Dimensions.h"
#include "krit/utils/Panic.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace krit {

using AssetId = unsigned int;

struct ImageProperties {
    IntDimensions dimensions;
    IntDimensions realDimensions;
    float scale = 1.0;

    ImageProperties(int w, int h, int rw, int rh)
        : dimensions(w, h), realDimensions(rw, rh), scale((float)rh / h) {}
};

union AssetProperties {
    ImageProperties img;
    bool _ = false;

    AssetProperties(int w, int h, int rw, int rh) : img(w, h, rw, rh) {}
    AssetProperties() : _(false) {}
};

struct AssetInfo {
    AssetId id = -1;
    AssetType type;
    std::string path;
    AssetProperties properties;
};

struct Assets {
    static void init();

    static const AssetInfo &byId(AssetId id);

    static const AssetInfo &byPath(const std::string &path);

    static const bool exists(const std::string &path) {
        return _byPath.find(path) != _byPath.end();
    }

private:
    static std::vector<AssetInfo> _assets;
    static std::unordered_map<std::string, std::vector<AssetId>> _byPath;
};

}

#endif
