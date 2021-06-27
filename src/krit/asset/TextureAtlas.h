#ifndef KRIT_ASSET_TEXTUREATLAS
#define KRIT_ASSET_TEXTUREATLAS

#include "krit/asset/AssetLoader.h"
#include "krit/render/ImageData.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Log.h"
#include "krit/Math.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace krit {

struct AssetCache;

struct TextureAtlas {
    std::unordered_map<std::string, ImageRegion> regions;

    TextureAtlas(const std::string &path);
    ImageRegion &getRegion(const std::string &region) {
        auto found = this->regions.find(region);
        if (found == this->regions.end()) {
            Log::fatal("couldn't find texture atlas region: %s\n", region.c_str());
        }
        return found->second;
    }
};

}

#endif
