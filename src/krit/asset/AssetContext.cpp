#include "krit/asset/AssetContext.h"
#include "krit/Assets.h"

namespace krit {

std::shared_ptr<void> AssetContext::get(const std::string &str) {
    return get(Assets::byPath(str));
}

std::shared_ptr<void> AssetContext::get(int id) {
    return get(Assets::byId(id));
}

PendingAssets &AssetContext::pendingAssets() {
    int i = 0;
    while (i < pending.size()) {
        auto &entry = pending[i];
        bool loaded = this->cache->isLoaded(entry.first, entry.second);
        if (loaded) {
            // remove from pending; we don't care about preserving order
            std::swap(entry, pending[pending.size() - 1]);
            pending.pop_back();
        }
        ++i;
    }
    return pending;
}

}
