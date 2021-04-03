#ifndef KRIT_ASSET_BITMAP_FONT
#define KRIT_ASSET_BITMAP_FONT

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

struct BitmapFont: public Font {
    std::vector<std::shared_ptr<ImageData>> pages;
    GlyphData glyphData[0x100];
    std::unordered_map<int64_t, int> kerningTable;
    AssetCache *cache;

    BitmapFont(AssetCache *cache, const char *path);
    ~BitmapFont() override = default;

    GlyphData getGlyph(int c) override {
        return this->glyphData[c];
    }

    std::shared_ptr<ImageData> &getPage(int i) override {
        return this->pages[i];
    }

    double kern(int32_t lastChar, int32_t thisChar) {
        int64_t key = (static_cast<int64_t>(lastChar) << 32) | thisChar;
        auto found = kerningTable.find(key);
        if (found != kerningTable.end()) {
            return found->second;
        }
        return 0;
    }
};

struct BitmapFontLoader: public AssetLoader {
    AssetCache *cache;

    BitmapFontLoader(AssetCache *cache): cache(cache) {}

    AssetType type() override { return BitmapFontAsset; }
    std::shared_ptr<void> loadAsset(const AssetInfo &info) override {
        std::shared_ptr<BitmapFont> font = std::make_shared<BitmapFont>(this->cache, info.path.c_str());
        return font;
    }
};

}

#endif
