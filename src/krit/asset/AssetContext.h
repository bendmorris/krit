#ifndef KRIT_ASSET_ASSET_CONTEXT
#define KRIT_ASSET_ASSET_CONTEXT

#include "krit/asset/AssetCache.h"
#include "krit/asset/BitmapFont.h"
#include "krit/asset/ImageLoader.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace krit {

typedef unordered_map<string, shared_ptr<void>> AssetMap;

/**
 * A single scoped collection of assets, backed by a shared AssetCache
 * instance.
 *
 * When this AssetContext is destroyed, its assets will be destroyed as well
 * unless there are other owners of the shared asset pointers.
 */
class AssetContext {
    public:
        AssetCache *cache;
        std::unordered_map<std::string, AssetMap> assets;

        AssetContext(AssetCache *cache): cache(cache) {}

        std::shared_ptr<void> get(const std::string &type, const std::string &id) {
            auto found = this->assets.find(type);
            if (found == this->assets.end()) {
                AssetMap assetMap;
                assetMap.reserve(16);
                found = (this->assets.insert(make_pair(type, assetMap))).first;
            }

            AssetMap &map = found->second;
            auto foundAsset = map.find(id);
            if (foundAsset == map.end()) {
                // not found; load the asset
                std::shared_ptr<void> result = this->cache->get(type, id);
                map.insert(make_pair(id, result));
                return result;
            } else {
                // found; return
                return foundAsset->second;
            }
        }

        std::shared_ptr<std::string> getText(const std::string &id) {
            return static_pointer_cast<std::string>(this->get("txt", id));
        }

        std::shared_ptr<ImageData> getImage(const std::string &id) {
            return static_pointer_cast<ImageData>(this->get("img", id));
        }

        std::shared_ptr<BitmapFont> getBitmapFont(const std::string &id) {
            return static_pointer_cast<BitmapFont>(this->get("bmf", id));
        }

        std::shared_ptr<TextureAtlas> getTextureAtlas(const std::string &id) {
            return static_pointer_cast<TextureAtlas>(this->get("tex", id));
        }
};

}

#endif
