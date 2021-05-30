#include "krit/asset/AssetCache.h"

namespace krit {

std::unordered_map<int, std::weak_ptr<void>> AssetCache::globalCache;

}
