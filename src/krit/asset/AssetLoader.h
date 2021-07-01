#ifndef KRIT_ASSET_ASSET_LOADER
#define KRIT_ASSET_ASSET_LOADER

#include <string_view>

namespace krit {

struct AssetInfo;

template <typename T> struct AssetLoader {
    static T *loadAsset(const AssetInfo &);
    static void unloadAsset(T *asset);
    static bool assetIsReady(T *asset);
    static bool assetIsReadyGeneric(void *asset) { return assetIsReady((T *)asset); }
};

#define DECLARE_ASSET_LOADER(T)                                                \
    template <> T *AssetLoader<T>::loadAsset(const AssetInfo &);               \
    template <> void AssetLoader<T>::unloadAsset(T *asset);                    \
    template <> bool AssetLoader<T>::assetIsReady(T *asset);                    \
    template <> bool AssetLoader<T>::assetIsReady(T *asset);
#define DECLARE_ASSET_LOADER2(T)                                               \
    struct T;                                                                  \
    DECLARE_ASSET_LOADER(T);

DECLARE_ASSET_LOADER2(ImageData)
DECLARE_ASSET_LOADER2(TextureAtlas)
DECLARE_ASSET_LOADER2(SkeletonBinaryData)
DECLARE_ASSET_LOADER2(Font)
DECLARE_ASSET_LOADER2(BitmapFont)

DECLARE_ASSET_LOADER(std::string_view)

#undef DECLARE_ASSET_LOADER

}

#endif
