#ifndef KRIT_ASSETS
#define KRIT_ASSETS

#include "krit/asset/AssetInfo.h"
#include "krit/utils/Panic.h"
#include <unordered_map>

namespace krit {

struct Assets {
    static const AssetInfo &byId(int id) {
        return _assets[id];
    }

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
