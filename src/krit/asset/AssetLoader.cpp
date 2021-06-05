#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include <cassert>
#include <string_view>

namespace krit {

template<> std::string_view *AssetLoader<std::string_view>::loadAsset(const AssetInfo &info) {
    // load asset as raw text
    assert(info.type == TextAsset);
    int length;
    char *content = IoRead::read(info.path, &length);
    std::string_view *slice = new std::string_view(content, length);
    return slice;
}

template<> void AssetLoader<std::string_view>::unloadAsset(std::string_view *slice) {
    IoRead::free((char*)(void*)slice->data());
    delete slice;
}

}
