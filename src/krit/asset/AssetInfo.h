#ifndef KRIT_ASSET_ASSETINFO
#define KRIT_ASSET_ASSETINFO

#include "krit/asset/AssetType.h"
#include "krit/math/Dimensions.h"
#include "krit/utils/Panic.h"
#include <string>
#include <unordered_map>

namespace krit {

enum AssetId : int;
enum AssetRoot : int;

struct ImageProperties {
    IntDimensions dimensions;
    IntDimensions realDimensions;
    float scale = 1.0;

    ImageProperties(int w, int h, int rw, int rh) : dimensions(w, h), realDimensions(rw, rh), scale((float)rh / h) {}
};

union AssetProperties {
    ImageProperties img;
    bool _ = false;

    AssetProperties(int w, int h, int rw, int rh) : img(w, h, rw, rh) {}
    AssetProperties() : _(false) {}
};

struct AssetInfo {
    AssetId id;
    AssetType type;
    std::string path;
    AssetProperties properties;
};

struct Assets {
    static const AssetInfo &byId(AssetId id);

    static const AssetInfo &byPath(const std::string &path) {
        auto found = _byPath.find(path);
        if (found == _byPath.end()) {
            panic("unrecognized asset path: %s\n", path.c_str());
        }
        return byId((AssetId)found->second);
    }

    static const bool exists(const std::string &path) {
        return _byPath.find(path) != _byPath.end();
    }

    static void enableRoot(AssetRoot root);
    static void disableRoot(AssetRoot root);

private:
    static const AssetInfo _assets[];
    static std::unordered_map<std::string, int> _byPath;
};

}

#endif
