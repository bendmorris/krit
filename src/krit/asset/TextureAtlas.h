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

using namespace std;
using namespace krit;

namespace krit {

class AssetCache;

class TextureAtlas {
    public:
        unordered_map<string, ImageRegion> regions;

        TextureAtlas(AssetCache *cache, const string &path);
        ImageRegion &getRegion(const string &region) {
            return this->regions.find(region)->second;
        }
};

class TextureAtlasLoader: public AssetLoader {
    public:
        AssetCache *cache;

        TextureAtlasLoader(AssetCache *cache): cache(cache) {}

        string assetType() override { return "tex"; }
        shared_ptr<void> loadAsset(string id) override {
            shared_ptr<TextureAtlas> font = make_shared<TextureAtlas>(this->cache, id);
            return font;
        }
};

}

#endif
