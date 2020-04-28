#include "krit/asset/AssetCache.h"
#include <fstream>
#include <sstream>
#include <memory>

std::shared_ptr<void> TextLoader::loadAsset(const std::string &id) {
    std::ifstream tinput(id);
    std::shared_ptr<std::stringstream> buffer = std::make_shared<std::stringstream>();
    *buffer << tinput.rdbuf();
    return buffer;
}

std::shared_ptr<void> AssetCache::get(const std::string &type, const std::string &id) {
    auto found = assets.find(type);
    AssetCacheMap &map = found->second;
    auto asset = map.find(id);
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
    auto foundLoader = loaders.find(type);
    AssetLoader *loader = foundLoader->second;
    std::shared_ptr<void> result = loader->loadAsset(id);
    std::weak_ptr<void> weak = result;
    map.insert(make_pair(id, weak));
    return result;
}
