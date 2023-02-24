#include "krit/asset/AssetCache.h"

namespace krit {

template <int A> static void bumpPreserved(AssetCache &cache) {
    for (auto &it : std::get<A>(cache.preserved)) {
        if (it.asset.use_count() == 1) {
            // we have the only reference to this
            ++it.unusedFrames;
            // Log::debug("unused asset this frame: %s for %zu frames", it.id.c_str(), it.unusedFrames);
        } else {
            it.unusedFrames = 0;
            // Log::debug("asset %s has use count of %zu", it.id.c_str(), it.asset.use_count());
        }
    }
    bumpPreserved<A + 1>(cache);
}

template <> void bumpPreserved<AssetTypeCount>(AssetCache &cache) {}

void AssetCache::update() {
    // for (size_t i = 0; i < assetRequests.size(); ++i) {
    //     auto &request = assetRequests[i];
    //     auto found = cache.find(request.id);
    //     if (found != cache.end()) {
    //         std::shared_ptr<void> asset = found->second.lock();
    //         if (asset && request.ready(asset.get())) {
    //             request.callback();
    //             assetRequests.erase(assetRequests.begin() + i);
    //             --i;
    //         }
    //     }
    // }
    bumpPreserved<0>(*this);
}

}
