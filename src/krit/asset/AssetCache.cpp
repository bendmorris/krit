#include "krit/asset/AssetCache.h"
#include "krit/Assets.h"
#include <memory>

std::shared_ptr<void> AssetCache::get(const std::string &path) {
    return get(Assets::byPath(path));
}

std::shared_ptr<void> AssetCache::get(const AssetInfo &info) {
    auto found = assets.find(info.type);
    AssetCacheMap &map = found->second;
    auto asset = map.find(info.id);
    if (asset != map.end()) {
        std::weak_ptr<void> weak = asset->second;
        std::shared_ptr<void> result = weak.lock();
        if (result) {
            // we still have a live reference to this asset
            return result;
        }
        // otherwise we have an expired weak_ptr, so reload
    }
    // load the asset, store a weak_ptr, and return a shared_ptr
    auto foundLoader = loaders.find(info.type);
    AssetLoader *loader = foundLoader->second;
    std::shared_ptr<void> result = loader->loadAsset(info);
    std::weak_ptr<void> weak = result;
    map.insert(std::make_pair(info.id, weak));
    return result;
}
