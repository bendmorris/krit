#ifndef KRIT_ASSET_TEXTUREATLAS
#define KRIT_ASSET_TEXTUREATLAS

#include "krit/asset/AssetLoader.h"
#include "krit/render/ImageData.h"
#include "krit/render/ImageRegion.h"
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

    TextureAtlas(AssetCache *cache, const std::string &path);
    ImageRegion &getRegion(const std::string &region) {
        return this->regions.find(region)->second;
    }
};

struct TextureAtlasLoader: public AssetLoader {
    AssetCache *cache;

    TextureAtlasLoader(AssetCache *cache): cache(cache) {}

    AssetType type() override { return AtlasAsset; }
    std::shared_ptr<void> loadAsset(const AssetInfo &info) override {
        std::shared_ptr<TextureAtlas> atlas = std::make_shared<TextureAtlas>(this->cache, info.path);
        return atlas;
    }
};

}

#endif
