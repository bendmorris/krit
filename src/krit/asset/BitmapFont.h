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

struct BitmapGlyphData {
    int page = 0;
    int id = 0;
    IntRectangle rect;
    IntPoint offset;
    int xAdvance = 0;

    BitmapGlyphData() {}
};

struct BitmapFontBase {
    int size = 0;
    int lineHeight = 0;

    virtual BitmapGlyphData getGlyph(int c) = 0;
    virtual std::shared_ptr<ImageData> &getPage(int i) = 0;
    virtual int kern(int32_t lastChar, int32_t thisChar) = 0;

    virtual ~BitmapFontBase() {}
};

struct BitmapFont: public BitmapFontBase {
    std::vector<std::shared_ptr<ImageData>> pages;
    BitmapGlyphData glyphData[0x100];
    std::unordered_map<int64_t, int> kerningTable;

    BitmapFont(const char *path);
    ~BitmapFont() override = default;

    BitmapGlyphData getGlyph(int c) override {
        return this->glyphData[c];
    }

    std::shared_ptr<ImageData> &getPage(int i) override {
        return this->pages[i];
    }

    int kern(int32_t lastChar, int32_t thisChar) {
        int64_t key = (static_cast<int64_t>(lastChar) << 32) | thisChar;
        auto found = kerningTable.find(key);
        if (found != kerningTable.end()) {
            return found->second;
        }
        return 0;
    }
};

}

#endif
