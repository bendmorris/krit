#include "krit/asset/AssetContext.h"
#include "krit/Assets.h"

std::shared_ptr<void> AssetContext::get(const std::string &str) {
    return get(Assets::byPath(str));
}
