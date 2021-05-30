#ifndef KRIT_ASSET_ASSET_LOADER
#define KRIT_ASSET_ASSET_LOADER

#include <memory>
#include <string>
#include "krit/asset/AssetInfo.h"
#include "krit/asset/AssetType.h"

namespace krit {

template <typename T> struct AssetLoader {
    static T *loadAsset(const AssetInfo&);
    static void unloadAsset(T *asset);
};

#define DECLARE_ASSET_LOADER(T) template<> T *AssetLoader<T>::loadAsset(const AssetInfo&); template<> void AssetLoader<T>::unloadAsset(T *asset);
#define DECLARE_ASSET_LOADER2(T) struct T; DECLARE_ASSET_LOADER(T);

DECLARE_ASSET_LOADER2(ImageData)
DECLARE_ASSET_LOADER2(TextureAtlas)
DECLARE_ASSET_LOADER2(SkeletonBinaryData)
DECLARE_ASSET_LOADER2(Font)
DECLARE_ASSET_LOADER2(BitmapFont)
DECLARE_ASSET_LOADER2(StringSlice)

#undef DECLARE_ASSET_LOADER

}

#endif
