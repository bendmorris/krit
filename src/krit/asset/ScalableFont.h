#ifndef KRIT_ASSET_SCALABLE_FONT
#define KRIT_ASSET_SCALABLE_FONT

#include "krit/asset/AssetLoader.h"
#include "krit/asset/Font.h"
#include "krit/render/ImageData.h"
#include "krit/Math.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace krit {

struct AssetCache;

// struct ScalableFont: public Font {
//     int lineHeight;
//     ImageData glyphCache;
//     AssetCache *cache;

//     ScalableFont(AssetCache *cache, const char *path);

//     GlyphData getGlyph(int c) override {
//         return this->glyphData[c];
//     }

//     std::shared_ptr<ImageData> &getPage(int i) override {
//         return this->pages[i];
//     }

//     double kern(int32_t)
// };

// struct ScalableFontLoader: public AssetLoader {
//     AssetCache *cache;

//     ScalableFontLoader(AssetCache *cache): cache(cache) {}

//     AssetType type() override { return ScalableFontAsset; }
//     std::shared_ptr<void> loadAsset(const AssetInfo &info) override {
//         std::shared_ptr<ScalableFont> font = std::make_shared<ScalableFont>(this->cache, info.path.c_str());
//         return font;
//     }
// };

}

#endif
