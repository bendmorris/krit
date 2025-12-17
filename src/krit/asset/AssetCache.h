#ifndef KRIT_ASSET_ASSETCACHE
#define KRIT_ASSET_ASSETCACHE

#include "krit/asset/AssetLoader.h"
// TODO: make this more generic
#include "krit/ecs/Utils.h"
#include "krit/utils/Log.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace krit {

struct SpineData;

struct AssetRequest {
    int id;
    bool (*ready)(void *id);
    std::function<void()> callback;
};

template <typename T> struct PreservedAsset {
    std::string id;
    std::shared_ptr<T> asset;
    size_t unusedFrames = 0;

    PreservedAsset() {}
    PreservedAsset(const std::string &id, std::shared_ptr<T> asset)
        : id(id), asset(asset) {}
};

template <typename T>
bool lru(const PreservedAsset<T> &a1, const PreservedAsset<T> &a2) {
    return a1.unusedFrames < a2.unusedFrames ||
           (a1.unusedFrames == a2.unusedFrames &&
            AssetLoader<T>::cost(a1.asset.get()) <
                AssetLoader<T>::cost(a2.asset.get()));
}

template <typename... AssetTypes> struct AssetCacheBase {
    std::tuple<std::unordered_map<std::string, std::weak_ptr<AssetTypes>>...>
        cache;
    std::tuple<std::vector<PreservedAsset<AssetTypes>>...> preserved;
    std::array<std::pair<size_t, size_t>, sizeof...(AssetTypes)> sizes;

    template <int A> void setLimit(size_t l) { std::get<A>(sizes).second = l; }

    template <int A>
    using SharedPtrT = decltype(std::get<A>(preserved)[0].asset);
    template <int A>
    using AssetT = typename std::remove_reference<
        decltype(*std::get<A>(preserved)[0].asset.get())>::type;
    template <int A> using AssetLoaderT = AssetLoader<AssetT<A>>;

    template <typename T> std::shared_ptr<T> get(const std::string &path) {
        return this->get<find_first<std::tuple<AssetTypes...>, T>::value>(path);
    }

    template <int A> SharedPtrT<A> get(const std::string &id) {
        auto &cache = std::get<A>(this->cache);
        auto &preserved = std::get<A>(this->preserved);
        auto &sizes = std::get<A>(this->sizes);
        {
            auto it = cache.find(id);
            if (it != cache.end()) {
                // we can reuse a global reference
                std::shared_ptr<void> v = it->second.lock();
                if (v) {
                    AREA_LOG_DEBUG("asset", "reuse asset: %s", id.c_str());
                    SharedPtrT<A> ptr = std::static_pointer_cast<AssetT<A>>(v);
                    return ptr;
                }
            }
        }
        // we need to load this asset
        AREA_LOG_INFO("asset", "load asset: %s", id.c_str());
        SharedPtrT<A> asset = AssetLoaderT<A>::loadAsset(id);
        if (!asset) {
            return nullptr;
        }
        size_t cost = AssetLoaderT<A>::cost(asset.get());

        cache[id] = std::weak_ptr<AssetT<A>>(asset);

        auto &it = sizes;
        it.first += cost;
        if (it.second > 0 && it.first > it.second) {
            AREA_LOG_DEBUG("asset",
                           "we need space (%i); need %zu, %zu total, %zu limit",
                           A, cost, it.first, it.second);
            // purge assets to make room
            std::sort(preserved.begin(), preserved.end(), lru<AssetT<A>>);
            while (it.first > it.second) {
                if (preserved.size() > 0) {
                    AREA_LOG_DEBUG(
                        "asset",
                        "I have a preserved asset in the back, %s, with "
                        "unused count %zu",
                        preserved.back().id.c_str(),
                        preserved.back().unusedFrames);
                }
                if (preserved.size() > 0 &&
                    preserved.back().asset.use_count() == 1 &&
                    preserved.back().unusedFrames > 0) {
                    auto &dropped = preserved.back();
                    size_t reclaimed =
                        AssetLoaderT<A>::cost(dropped.asset.get());
                    AREA_LOG_INFO("asset", "drop asset: %s (reclaimed %zu)",
                                  preserved.back().id.c_str(), reclaimed);
                    it.first -= reclaimed;
                    preserved.pop_back();
                } else {
                    break;
                }
            }
        }

        preserved.push_back(PreservedAsset<AssetT<A>>(id, asset));

        return asset;
    }
};

struct AssetCache
    : public AssetCacheBase<ImageData, TextureAtlas, Font, std::string,
                            SpineData, AudioData, ParticleEffect> {
    void update();
};

}

#endif
