#ifndef KRIT_ASSET_FONT
#define KRIT_ASSET_FONT

#include "krit/asset/AssetLoader.h"
#include "krit/math/Point.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Panic.h"
#include <cstddef>
#include <memory>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

struct hb_blob_t;
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
    unsigned int border;

    GlyphSize(Font *font, char32_t glyphIndex, unsigned int size,
              unsigned int border)
        : font(font), glyphIndex(glyphIndex), size(size), border(border) {}
    bool operator==(const GlyphSize &other) const {
        return font == other.font && glyphIndex == other.glyphIndex &&
               size == other.size && border == other.border;
    }
};

struct GlyphSizeHash {
    std::size_t operator()(const GlyphSize &size) const {
        return std::hash<char32_t>()(size.glyphIndex) ^
               std::hash<unsigned int>()(size.size) ^
               std::hash<unsigned int>()(size.border);
    }
};

struct GlyphData {
    ImageRegion region;
    Point offset;

    GlyphData() {}
    GlyphData(ImageRegion region, float x, float y)
        : region(region), offset(x, y) {}
};

struct ColumnData {
    int width = 0;
    int height = 0;

    ColumnData(int width, int height) : width(width), height(height) {}
};

struct GlyphCache {
    static const int SIZE_PRECISION = 8;
    static const int PADDING = 4;
    static const int CACHE_TEXTURE_SIZE = 2048;

    std::unordered_map<GlyphSize, GlyphData, GlyphSizeHash> glyphs;
    std::vector<ColumnData> columns;
    std::vector<GlyphSize> pending;
    std::shared_ptr<ImageData> img;
    std::unique_ptr<uint8_t[]> pixelData;

    GlyphCache() {}

    GlyphData *getGlyph(Font *font, char32_t codePoint, unsigned int size,
                        unsigned int border = 0);
    void createTexture();
    void commitChanges();
};

struct FontManager {
    GlyphCache glyphCache, nextGlyphCache;
    std::unordered_map<std::string, std::shared_ptr<Font>> fontRegistry;

    FontManager();
    ~FontManager();

    void commit();
    void flush();
    std::shared_ptr<Font> registerFont(const std::string &name, const std::string &path);

    Font *getFont(const std::string &name) {
        Font *font = fontRegistry[name].get();
        if (!font) {
            panic("missing font: %s\n", name.c_str());
        }
        return font;
    }

    GlyphData &getGlyph(Font *font, char32_t glyphId, unsigned int size,
                        unsigned int border = 0);
};

struct Font {
    Font(const std::string &path, const std::string &fontData);
    Font() {}
    ~Font();
    Font(Font &&other) {
        this->face = other.face;
        this->font = other.font;
        this->ftFace = other.ftFace;
        this->fontData = std::move(other.fontData);
        this->path = std::move(other.path);
        other.face = nullptr;
        other.font = nullptr;
        other.ftFace = nullptr;
    }

    bool ligatures = true;

    std::string path;
    void shape(hb_buffer_t *buf, size_t pointSize);
    void commitChanges();
    void flushCache();

private:
    hb_blob_t *blob = nullptr;
    hb_face_t *face = nullptr;
    hb_font_t *font = nullptr;
    void *ftFace = nullptr;
    std::string fontData;
    friend struct GlyphCache;
    friend struct Text;
    friend struct TextParser;
};

}

#endif
