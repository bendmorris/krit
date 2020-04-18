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

using namespace std;
using namespace krit;

namespace krit {

class AssetCache;

struct GlyphData {
    int page = 0;
    int id = 0;
    IntRectangle rect;
    IntPoint offset;
    int xAdvance = 0;

    GlyphData() {}
};

class BitmapFont {
    public:
        int size;
        int lineHeight;
        vector<shared_ptr<ImageData>> pages;
        GlyphData glyphData[0x100];
        unordered_map<int64_t, int> kerningTable;
        AssetCache *cache;

        BitmapFont(AssetCache *cache, const char *path);

        GlyphData &getGlyph(int c) {
            return this->glyphData[c];
        }

        shared_ptr<ImageData> &getPage(int i) {
            return this->pages[i];
        }
};

class BitmapFontLoader: public AssetLoader {
    public:
        AssetCache *cache;

        BitmapFontLoader(AssetCache *cache): cache(cache) {}

        string assetType() override { return "bmf"; }
        shared_ptr<void> loadAsset(string id) override {
            shared_ptr<BitmapFont> font = make_shared<BitmapFont>(this->cache, id.c_str());
            return font;
        }
};

}

#endif
