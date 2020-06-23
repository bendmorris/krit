#ifndef KRIT_ASSET_ASSET_CONTEXT
#define KRIT_ASSET_ASSET_CONTEXT

#include "krit/asset/AssetCache.h"
#include "krit/asset/AssetType.h"
#include "krit/asset/BitmapFont.h"
#include "krit/asset/ImageLoader.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace krit {

typedef std::unordered_map<int, std::shared_ptr<void>> AssetMap;
typedef std::vector<std::pair<AssetType, std::shared_ptr<void>>> PendingAssets;

/**
 * A single scoped collection of assets, backed by a shared AssetCache
 * instance.
 *
 * When this AssetContext is destroyed, its assets will be destroyed as well
 * unless there are other owners of the shared asset pointers.
 */
struct AssetContext {
    AssetCache &cache;
    AssetMap assets;
    PendingAssets pending;

    AssetContext(AssetCache &cache): cache(cache) {}

    std::shared_ptr<void> get(const std::string &str);
    std::shared_ptr<void> get(int id);
    std::shared_ptr<void> get(const krit::AssetInfo &info) {
        auto foundAsset = assets.find(info.id);
        if (foundAsset == assets.end()) {
            // not found; load the asset
            std::shared_ptr<void> result = this->cache.get(info);
            if (!this->cache.isLoaded(info.type, result)) {
                pending.push_back(std::make_pair(info.type, result));
            }
            assets.insert(std::make_pair(info.id, result));
            return result;
        } else {
            // found; return
            return foundAsset->second;
        }
    }

    PendingAssets &pendingAssets();

    template <typename Arg> std::shared_ptr<std::string> getText(const Arg &a) {
        return std::static_pointer_cast<std::string>(this->get(a));
    }

    template <typename Arg> std::shared_ptr<ImageData> getImage(const Arg &a) {
        return std::static_pointer_cast<ImageData>(this->get(a));
    }

    template <typename Arg> std::shared_ptr<BitmapFont> getBitmapFont(const Arg &a) {
        return std::static_pointer_cast<BitmapFont>(this->get(a));
    }

    template <typename Arg> std::shared_ptr<TextureAtlas> getTextureAtlas(const Arg &a) {
        return std::static_pointer_cast<TextureAtlas>(this->get(a));
    }

    void clear() {
        assets.clear();
        pending.clear();
    }
};

}

#endif
