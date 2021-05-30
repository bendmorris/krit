#ifndef KRIT_ASSET_FONT
#define KRIT_ASSET_FONT

#include "krit/math/Point.h"
#include "krit/math/ScaleFactor.h"
#include "krit/math/Rectangle.h"
#include "krit/render/ImageData.h"
#include "krit/utils/Color.h"
#include <memory>
#include <unordered_map>

struct hb_face_t;
struct hb_font_t;
struct hb_buffer_t;

namespace krit {

struct GlyphData {
    int page = 0;
    int id = 0;
    IntRectangle rect;
    IntPoint offset;
    int xAdvance = 0;

    GlyphData() {}
};

struct GlyphCache {
    
};

struct Font {
    static void init();

    static const int SCALE = 8;

    static std::unordered_map<std::string, Font*> fontRegistry;
    static Font *getFont(const std::string &name) { return fontRegistry[name]; }
    static void registerFont(const std::string &name);

    Font(const char *fontData, size_t fontDataLen);

    size_t lineHeight(size_t pointSize);
    ImageData getGlyph(uint32_t glyphId);
    void shape(hb_buffer_t *buf, size_t pointSize);

    private:
        hb_face_t *face;
        hb_font_t *font;
        void *ftFace;
        void *fontData;
};

}

#endif
