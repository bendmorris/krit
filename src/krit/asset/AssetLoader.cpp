#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/utils/Slice.h"
#include <cassert>

namespace krit {

template<> StringSlice *AssetLoader<StringSlice>::loadAsset(const AssetInfo &info) {
    // load asset as raw text
    assert(info.type == TextAsset);
    int length;
    char *content = IoRead::read(info.path, &length);
    StringSlice *slice = new StringSlice(content, length);
    return slice;
}

template<> void AssetLoader<StringSlice>::unloadAsset(StringSlice *slice) {
    IoRead::free(slice->data);
    delete slice;
}

}
