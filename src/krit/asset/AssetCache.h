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

namespace krit {

static const std::string TEXT_TYPE = "txt";

struct TextLoader: public AssetLoader {
    const std::string &assetType() override { return TEXT_TYPE; }
    std::shared_ptr<void> loadAsset(const std::string &id) override;
    TextLoader() {}
};

typedef std::unordered_map<std::string, std::weak_ptr<void>> AssetCacheMap;

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
struct AssetCache {
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
        const std::string &type = loader->assetType();
        this->loaders.insert(make_pair(type, loader));
        AssetCacheMap assetMap;
        assetMap.reserve(16);
        this->assets.insert(make_pair(type, assetMap));
    }

    bool registered(const std::string &id) {
        return this->loaders.find(id) != this->loaders.end();
    }

    std::shared_ptr<void> get(const std::string &type, const std::string &id);

    private:
        std::unordered_map<std::string, AssetCacheMap> assets;
        std::unordered_map<std::string, AssetLoader*> loaders;
};

}

#endif
