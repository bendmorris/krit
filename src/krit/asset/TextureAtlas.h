#ifndef KRIT_ASSET_TEXTUREATLAS
#define KRIT_ASSET_TEXTUREATLAS

#include "krit/render/ImageRegion.h"
#include "krit/utils/Log.h"
#include <string>
#include <unordered_map>
#include <utility>

namespace krit {

struct ImageRegion;

struct TextureAtlas {
    std::unordered_map<std::string, ImageRegion> regions;

    TextureAtlas(const std::string &path);

    ImageRegion &getRegion(const std::string &region) {
        auto found = this->regions.find(region);
        if (found == this->regions.end()) {
            Log::fatal("couldn't find texture atlas region: %s\n",
                       region.c_str());
            return ImageRegion::empty;
        }
        return found->second;
    }

    bool hasRegion(const std::string &region) {
        return regions.find(region) != regions.end();
    }
};

}

#endif
