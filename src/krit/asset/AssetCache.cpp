#include "krit/asset/AssetCache.h"
#include <fstream>
#include <sstream>
#include <memory>

shared_ptr<void> TextLoader::loadAsset(string id) {
    ifstream tinput(id);
    shared_ptr<stringstream> buffer = make_shared<stringstream>();
    *buffer << tinput.rdbuf();
    return buffer;
}

shared_ptr<void> AssetCache::get(string type, string id) {
    auto found = assets.find(type);
    AssetCacheMap &map = found->second;
    auto asset = map.find(id);
    if (asset != map.end()) {
        weak_ptr<void> weak = asset->second;
        shared_ptr<void> result = weak.lock();
        if (result) {
            // we still have a live reference to this asset
            return result;
        }
        // otherwise we have an expired weak_ptr, so reload
    }
    // load the asset, store a weak_ptr, and return a shared_ptr
    auto foundLoader = loaders.find(type);
    AssetLoader *loader = foundLoader->second;
    shared_ptr<void> result = loader->loadAsset(id);
    weak_ptr<void> weak = result;
    map.insert(make_pair(id, weak));
    return result;
}
