#ifndef KRIT_ASSET_ASSET_LOADER
#define KRIT_ASSET_ASSET_LOADER

#include "krit/asset/AssetType.h"
#include "krit/math/Dimensions.h"
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace krit {

template <typename T> struct AssetLoader {
    static AssetType type();
    static std::shared_ptr<T> loadAsset(const std::string &path);
    static bool assetIsReady(T *asset);
    static bool assetIsReadyGeneric(void *asset) {
        return assetIsReady((T *)asset);
    }
    static size_t cost(T *asset) { return 1; }
};

#define DECLARE_ASSET_LOADER(T)                                                \
    template <> AssetType AssetLoader<T>::type();                              \
    template <>                                                                \
    std::shared_ptr<T> AssetLoader<T>::loadAsset(const std::string &path);     \
    template <> bool AssetLoader<T>::assetIsReady(T *asset);                   \
    template <> size_t AssetLoader<T>::cost(T *asset);
#define DECLARE_ASSET_LOADER2(T)                                               \
    struct T;                                                                  \
    DECLARE_ASSET_LOADER(T);

DECLARE_ASSET_LOADER2(ImageData)
DECLARE_ASSET_LOADER2(TextureAtlas)
DECLARE_ASSET_LOADER2(SkeletonBinaryData)
DECLARE_ASSET_LOADER2(Font)
DECLARE_ASSET_LOADER2(SoundData)
DECLARE_ASSET_LOADER2(MusicData)
DECLARE_ASSET_LOADER2(ParticleEffect)
DECLARE_ASSET_LOADER(std::string_view)

struct ImageLoader {
    static void parseManifest();
};

#undef DECLARE_ASSET_LOADER

}

#endif
