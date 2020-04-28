#ifndef KRIT_ASSET_BITMAP_FONT
#define KRIT_ASSET_BITMAP_FONT

#include "krit/asset/AssetLoader.h"
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

struct GlyphData {
    int page = 0;
    int id = 0;
    IntRectangle rect;
    IntPoint offset;
    int xAdvance = 0;

    GlyphData() {}
};

struct BitmapFont {
    int size;
    int lineHeight;
    std::vector<std::shared_ptr<ImageData>> pages;
    GlyphData glyphData[0x100];
    std::unordered_map<int64_t, int> kerningTable;
    AssetCache *cache;

    BitmapFont(AssetCache *cache, const char *path);

    GlyphData &getGlyph(int c) {
        return this->glyphData[c];
    }

    std::shared_ptr<ImageData> &getPage(int i) {
        return this->pages[i];
    }
};

static const std::string BMF_TYPE = "bmf";

struct BitmapFontLoader: public AssetLoader {
    AssetCache *cache;

    BitmapFontLoader(AssetCache *cache): cache(cache) {}

    const std::string &assetType() override { return BMF_TYPE; }
    std::shared_ptr<void> loadAsset(const std::string &id) override {
        std::shared_ptr<BitmapFont> font = std::make_shared<BitmapFont>(this->cache, id.c_str());
        return font;
    }
};

}

#endif
