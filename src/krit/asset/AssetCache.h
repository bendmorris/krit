#ifndef KRIT_ASSET_ASSET_CACHE
#define KRIT_ASSET_ASSET_CACHE

#include "krit/asset/AssetLoader.h"
#include "krit/asset/BitmapFont.h"
#include "krit/asset/ImageLoader.h"
#include "krit/asset/TextureAtlas.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

using namespace std;
using namespace krit;

namespace krit {

class TextLoader: public AssetLoader {
    public:
        string assetType() override { return "txt"; }
        shared_ptr<void> loadAsset(string id) override;
        TextLoader() {}
};

typedef unordered_map<string, weak_ptr<void>> AssetCacheMap;

/**
 * A single AssetCache is used per Engine to load and manage assets. The cache
 * is usually not used directly, but through an AssetContext, which manages the
 * set of assets needed for a given scope.
 *
 * When an AssetContext doesn't contain a needed asset, it will request one
 * from the AssetCache. The AssetCache stores weak pointers to previously
 * loaded assets, so it can return a reference to the existing asset if still
 * live, or load and cache a weak pointer otherwise.
 */
class AssetCache {
    public:
        TextLoader txtLoader;
        ImageLoader imgLoader;
        BitmapFontLoader bmfLoader;
        TextureAtlasLoader atlasLoader;

        AssetCache(): bmfLoader(this), atlasLoader(this) {
            this->registerLoader(&this->txtLoader);
            this->registerLoader(&this->imgLoader);
            this->registerLoader(&this->bmfLoader);
            this->registerLoader(&this->atlasLoader);
        }

        void registerLoader(AssetLoader *loader) {
            string type = loader->assetType();
            this->loaders.insert(make_pair(type, loader));
            AssetCacheMap assetMap;
            assetMap.reserve(16);
            this->assets.insert(make_pair(type, assetMap));
        }

        bool registered(string &id) {
            return this->loaders.find(id) != this->loaders.end();
        }

        shared_ptr<void> get(string type, string id);

    private:
        unordered_map<string, AssetCacheMap> assets;
        unordered_map<string, AssetLoader*> loaders;
};

}

#endif
