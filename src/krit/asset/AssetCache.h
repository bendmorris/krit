#ifndef KRIT_ASSET_ASSETCACHE
#define KRIT_ASSET_ASSETCACHE

#include "krit/Assets.h"
#include "krit/asset/AssetInfo.h"
#include "krit/asset/AssetLoader.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace krit {

/**
 * An AssetCache is a collection of assets with the same lifetime; it holds
 * references to these assets and will prevent them from being destroyed before
 * the cache.
 *
 * A global cache is responsible for actual loading of assets, so if multiple
 * AssetCaches concurrently load the same asset, they'll receive a pointer to
 * the same loaded instance; only when all of those caches are destroyed will
 * the asset be unloaded.
 */
struct AssetCache {
    static std::unordered_map<int, std::weak_ptr<void>> globalCache;

    std::unordered_map<int, std::shared_ptr<void>> cache;

    template <typename T> std::shared_ptr<T> get(const std::string &path) {
        return this->get<T>(Assets::byPath(path));
    }
    template <typename T> std::shared_ptr<T> get(const AssetId id) {
        return this->get<T>(Assets::byId(id));
    }
    template <typename T> std::shared_ptr<T> get(int id) {
        return this->get<T>(Assets::byId((AssetId)id));
    }
    template <typename T> std::shared_ptr<T> get(const AssetInfo &info) {
        int id = info.id;
        {
            auto it = cache.find(id);
            if (it != cache.end()) {
                // we have a live reference to this resource
                return std::static_pointer_cast<T>(it->second);
            }
        }
        {
            auto it = globalCache.find(id);
            if (it != globalCache.end()) {
                // we can reuse a global reference
                std::shared_ptr<void> v = it->second.lock();
                if (v) {
                    std::shared_ptr<T> ptr = std::static_pointer_cast<T>(v);
                    cache[id] = ptr;
                    return ptr;
                }
            }
        }
        // we need to load this asset
        std::shared_ptr<T> asset =
            std::shared_ptr<T>(AssetLoader<T>::loadAsset(info), [&](T *ptr) {
                AssetLoader<T>::unloadAsset(ptr);
            });
        globalCache[id] = std::weak_ptr<void>(asset);
        cache[id] = asset;
        return asset;
    }
};

}

#endif
