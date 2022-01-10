#ifndef KRIT_ASSET_ASSETCACHE
#define KRIT_ASSET_ASSETCACHE

#include "krit/asset/AssetInfo.h"
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

enum AssetId : int;
struct SpineData;

struct AssetRequest {
    int id;
    bool (*ready)(void *id);
    std::function<void()> callback;
};

template <typename T> struct PreservedAsset {
    AssetId id;
    std::shared_ptr<T> asset;
    size_t unusedFrames = 0;

    PreservedAsset() {}
    PreservedAsset(AssetId id, std::shared_ptr<T> asset)
        : id(id), asset(asset) {}
};

template <typename T>
bool lru(const PreservedAsset<T> &a1, const PreservedAsset<T> &a2) {
    return a1.unusedFrames < a2.unusedFrames;
}

template <typename... AssetTypes> struct AssetCacheBase {
    std::tuple<std::unordered_map<int, std::weak_ptr<AssetTypes>>...> cache;
    std::tuple<std::vector<PreservedAsset<AssetTypes>>...> preserved;
    std::array<std::pair<size_t, size_t>, sizeof...(AssetTypes)> sizes;

    template <int A> void setLimit(size_t l) { std::get<A>(sizes).second = l; }

    template <int A>
    using SharedPtrT = decltype(std::get<A>(preserved)[0].asset);
    template <int A>
    using AssetT = typename std::remove_reference<decltype(
        *std::get<A>(preserved)[0].asset.get())>::type;
    template <int A> using AssetLoaderT = AssetLoader<AssetT<A>>;

    template <typename T> std::shared_ptr<T> get(const std::string &path) {
        return this->get<find_first<std::tuple<AssetTypes...>, T>::value>(path);
    }

    template <typename T> std::shared_ptr<T> get(const AssetId id) {
        return this->get<find_first<std::tuple<AssetTypes...>, T>::value>(id);
    }

    template <typename T> std::shared_ptr<T> get(int id) {
        return this->get<find_first<std::tuple<AssetTypes...>, T>::value>(id);
    }

    template <typename T> std::shared_ptr<T> get(const AssetInfo &info) {
        return this->get<find_first<std::tuple<AssetTypes...>, T>::value>(info);
    }

    template <int A> SharedPtrT<A> get(const std::string &path) {
        return this->get<A>(Assets::byPath(path));
    }
    template <int A> SharedPtrT<A> get(const AssetId id) {
        return this->get<A>(Assets::byId(id));
    }
    template <int A> SharedPtrT<A> get(int id) {
        return this->get<A>(Assets::byId((AssetId)id));
    }
    template <int A> SharedPtrT<A> get(const AssetInfo &info) {
        int id = info.id;
        auto &cache = std::get<A>(this->cache);
        auto &preserved = std::get<A>(this->preserved);
        auto &sizes = std::get<A>(this->sizes);
        {
            auto it = cache.find(id);
            if (it != cache.end()) {
                // we can reuse a global reference
                std::shared_ptr<void> v = it->second.lock();
                if (v) {
                    Log::info("reuse asset: %s", info.path.c_str());
                    SharedPtrT<A> ptr = std::static_pointer_cast<AssetT<A>>(v);
                    return ptr;
                }
            }
        }
        // we need to load this asset
        Log::info("load asset: %s", info.path.c_str());
        SharedPtrT<A> asset = AssetLoaderT<A>::loadAsset(info);
        size_t cost = AssetLoaderT<A>::cost(asset.get());

        cache[id] = std::weak_ptr<AssetT<A>>(asset);
        preserved.push_back(PreservedAsset<AssetT<A>>((AssetId)id, asset));

        auto &it = sizes;
        it.first += cost;
        if (it.second > 0 && it.first > it.second) {
            // purge assets to make room
            std::sort(preserved.begin(), preserved.end(), lru<AssetT<A>>);
            while (it.first > it.second) {
                if (preserved.size() > 0 && preserved.back().asset.use_count() == 1 && preserved.back().unusedFrames > 0) {
                    auto &dropped = preserved.back();
                    Log::info("drop asset: %s",
                              Assets::byId(dropped.id).path.c_str());
                    it.first -= AssetLoaderT<A>::cost(dropped.asset.get());
                    preserved.pop_back();
                } else {
                    break;
                }
            }
        }
        return asset;
    }
};

struct AssetCache
    : public AssetCacheBase<ImageData, TextureAtlas, Font, std::string_view,
                            SpineData, SoundData, MusicData> {
    // std::vector<AssetRequest> assetRequests;

    // template <typename T>
    // void requestAsset(const AssetId id, std::function<void()> callback) {
    //     assetRequests.emplace_back(
    //         (AssetRequest){.id = id,
    //                        .ready = AssetLoader<T>::assetIsReadyGeneric,
    //                        .callback = callback});
    // }

    void update();

    // template <typename T> std::shared_ptr<T> load(const AssetId id) {
    //     return load<T>(Assets::byId(id));
    // }

    // template <typename T> std::shared_ptr<T> load(const std::string &path) {
    //     return load<T>(Assets::byPath(path));
    // }

    // template <typename T> std::shared_ptr<T> load(const AssetInfo &info) {
    //     return AssetLoader<T>::loadAsset(info);
    // }
};

}

#endif
