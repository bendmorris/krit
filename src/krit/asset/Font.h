#ifndef KRIT_ASSET_FONT
#define KRIT_ASSET_FONT

#include "krit/math/Point.h"
#include "krit/math/ScaleFactor.h"
#include "krit/math/Rectangle.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <cassert>

struct hb_face_t;
struct hb_font_t;
struct hb_buffer_t;

namespace krit {

struct Font;
struct Text;
struct TextParser;
enum AssetId: int;

struct GlyphSize {
    Font *font;
    uint32_t glyphIndex;
    float size;

    GlyphSize(Font *font, uint32_t glyphIndex, float size): font(font), glyphIndex(glyphIndex), size(size) {}
    bool operator==(const GlyphSize &other) const { return font == other.font && glyphIndex == other.glyphIndex && size == other.size; }
};

struct GlyphSizeHash {
    std::size_t operator()(const GlyphSize &size) const {
        return std::hash<uint32_t>()(size.glyphIndex) ^ std::hash<float>()(size.size);
    }
};

struct GlyphData {
    ImageRegion region;
    Point offset;

    GlyphData() {}
    GlyphData(ImageRegion region, double x, double y): region(region), offset(x, y) {}
};
 
struct ColumnData {
    int width = 0;
    int height = 0;

    ColumnData(int width, int height): width(width), height(height) {}
};

struct Font;

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

    GlyphData *getGlyph(Font *font, uint32_t codePoint, float size);
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
    GlyphData &getGlyph(uint32_t glyphId, float size);
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
