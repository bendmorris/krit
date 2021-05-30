#include "krit/asset/Font.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/utils/Panic.h"
#include "harfbuzz/hb.h"
#include <cassert>
#include "freetype2/ft2build.h"
#include FT_FREETYPE_H

namespace krit {

static FT_Library ftLibrary;

void Font::init() {
    int error = FT_Init_FreeType(&ftLibrary);
    if (error) {
        panic("failed to initialize freetype: %i\n", error);
    }
}

std::unordered_map<std::string, Font*> Font::fontRegistry;

void Font::registerFont(const std::string &name) {
    // TODO
}

Font::Font(const char *fontData, size_t fontDataLen) {
    this->fontData = (void*)fontData;
    // harfbuzz face initialization
    hb_blob_t *blob = hb_blob_create(fontData, fontDataLen, HB_MEMORY_MODE_READONLY, nullptr, nullptr);
    face = hb_face_create(blob, 0);
    font = hb_font_create(face);
    hb_font_set_scale(font, SCALE, SCALE);
    // freetype face initialization
    int error = FT_New_Memory_Face(ftLibrary, (const FT_Byte*)fontData, fontDataLen, 0, (FT_Face*)&ftFace);
    if (error) {
        panic("failed to initialize font");
    }
}

size_t Font::lineHeight(size_t pointSize) {
    hb_font_set_ptem(font, pointSize);
    hb_font_extents_t extents;
    hb_font_get_h_extents(font, &extents);
    return extents.line_gap / SCALE;
}

void Font::shape(hb_buffer_t *buf, size_t pointSize) {
    hb_font_set_ptem(font, pointSize);
    hb_shape(font, buf, nullptr, 0);
}

template<> Font *AssetLoader<Font>::loadAsset(const AssetInfo &info) {
    int length;
    char *content = IoRead::read(info.path, &length);
    Font *font = new Font(content, length);
    return font;
}

template<> void AssetLoader<Font>::unloadAsset(Font *font) {
    delete font;
}

}
