#ifndef KRIT_ASSET_FONT
#define KRIT_ASSET_FONT

#include <stdint.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <cstddef>
#include <string>
#include "krit/math/Point.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Panic.h"
#include "krit/Assets.h"

struct hb_face_t;
struct hb_font_t;
struct hb_buffer_t;

namespace krit {

struct Font;
struct ImageData;

struct GlyphSize {
    Font *font;
    char32_t glyphIndex;
    unsigned int size;

    GlyphSize(Font *font, char32_t glyphIndex, unsigned int size): font(font), glyphIndex(glyphIndex), size(size) {}
    bool operator==(const GlyphSize &other) const { return font == other.font && glyphIndex == other.glyphIndex && size == other.size; }
};

struct GlyphSizeHash {
    std::size_t operator()(const GlyphSize &size) const {
        return std::hash<char32_t>()(size.glyphIndex) ^ std::hash<unsigned int>()(size.size);
    }
};

struct GlyphData {
    ImageRegion region;
    Point offset;

    GlyphData() {}
    GlyphData(ImageRegion region, float x, float y): region(region), offset(x, y) {}
};
 
struct ColumnData {
    int width = 0;
    int height = 0;

    ColumnData(int width, int height): width(width), height(height) {}
};

struct GlyphCache {
    static const int SIZE_PRECISION = 8;
    static const int PADDING = 4;
    static const int CACHE_TEXTURE_SIZE = 2048;

    std::unordered_map<GlyphSize, GlyphData, GlyphSizeHash> glyphs;
    std::vector<ColumnData> columns;
    std::vector<GlyphSize> pending;
    std::shared_ptr<ImageData> img;
    uint8_t *pixelData = nullptr;

    GlyphCache() {}
    ~GlyphCache();

    GlyphData *getGlyph(Font *font, char32_t codePoint, unsigned int size);
    void createTexture();
    void commitChanges();
};

struct Font {
    static GlyphCache glyphCache, nextGlyphCache;

    static void init();
    static void commit();
    static void flush();

    static std::unordered_map<std::string, Font*> fontRegistry;
    static Font *getFont(const std::string &name) {
        Font *font = fontRegistry[name];
        if (!font) {
            panic("missing font: %s\n", name.c_str());
        }
        return font;
    }
    static void registerFont(const std::string &name, const std::string &path);
    static void registerFont(const std::string &name, AssetId id);

    Font(const std::string &path, const char *fontData, size_t fontDataLen);

    std::string path;
    GlyphData &getGlyph(char32_t glyphId, unsigned int size);
    void shape(hb_buffer_t *buf, size_t pointSize);
    void commitChanges();
    void flushCache();

    private:
        hb_face_t *face;
        hb_font_t *font;
        void *ftFace;
        void *fontData;
        friend struct GlyphCache;
        friend struct Text;
        friend struct TextParser;
};

}

#endif
