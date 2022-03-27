#include "krit/asset/AssetLoader.h"
#include "krit/asset/AssetType.h"
#include "krit/io/Io.h"
#include <cassert>
#include <string_view>

namespace krit {

struct TextDeleter {
    void operator()(std::string_view const *view) {
        IoRead::free(const_cast<char *>(view->data()));
        delete view;
    }
};

template <>
std::shared_ptr<std::string_view>
AssetLoader<std::string_view>::loadAsset(const std::string &path) {
    // load asset as raw text
    int length;
    char *content = IoRead::read(path, &length);
    std::string_view *view = new std::string_view(content, length);
    return std::shared_ptr<std::string_view>(view, TextDeleter());
}

template <>
bool AssetLoader<std::string_view>::assetIsReady(std::string_view *txt) {
    return true;
}

template <> AssetType AssetLoader<std::string_view>::type() {
    return TextAsset;
}

template <> size_t AssetLoader<std::string_view>::cost(std::string_view *txt) {
    return txt->size();
}

}
