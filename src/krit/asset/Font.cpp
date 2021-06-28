#include "krit/asset/Font.h"

#include <GL/glew.h>
#include <stdlib.h>
#include <cstring>
#include <tuple>
#include <utility>

#include "krit/Assets.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/render/ImageData.h"
#include "krit/utils/Panic.h"
#include "harfbuzz/hb.h"
#include "freetype/config/ftheader.h"
#include "freetype/ftimage.h"
#include "freetype/fttypes.h"
#include "krit/asset/AssetInfo.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Rectangle.h"

#include FT_FREETYPE_H

namespace krit {

GlyphCache Font::glyphCache, Font::nextGlyphCache;

template<> Font *AssetLoader<Font>::loadAsset(const AssetInfo &info) {
    int length;
    char *content = IoRead::read(info.path, &length);
    Font *font = new Font(info.path, content, length);
    return font;
}

template<> void AssetLoader<Font>::unloadAsset(Font *font) {
    delete font;
}

static FT_Library ftLibrary;
std::unordered_map<std::string, Font*> Font::fontRegistry;

void Font::init() {
    int error = FT_Init_FreeType(&ftLibrary);
    if (error) {
        panic("failed to initialize freetype: %i\n", error);
    }
}

void Font::commit() {
    if (glyphCache.img) glyphCache.commitChanges();
    if (nextGlyphCache.img) nextGlyphCache.commitChanges();
}

void Font::flush() {
    if (nextGlyphCache.img) {
        glyphCache = nextGlyphCache;
        nextGlyphCache = GlyphCache();
    }
}

void Font::registerFont(const std::string &name, AssetId id) {
    fontRegistry[name] = AssetLoader<Font>::loadAsset(Assets::byId(id));
}

void Font::registerFont(const std::string &name, const std::string &path) {
    fontRegistry[name] = AssetLoader<Font>::loadAsset(Assets::byPath(path));
}

Font::Font(const std::string &path, const char *fontData, size_t fontDataLen): path(path) {
    this->fontData = (void*)fontData;
    // harfbuzz face initialization
    hb_blob_t *blob = hb_blob_create(fontData, fontDataLen, HB_MEMORY_MODE_READONLY, nullptr, nullptr);
    face = hb_face_create(blob, 0);
    font = hb_font_create(face);
    hb_font_set_scale(font, 64, 64);
    // freetype face initialization
    int error = FT_New_Memory_Face(ftLibrary, (const FT_Byte*)fontData, fontDataLen, 0, (FT_Face*)&ftFace);
    if (error) {
        panic("failed to initialize font");
    }
}

void Font::shape(hb_buffer_t *buf, size_t pointSize) {
    hb_font_set_ppem(font, pointSize, pointSize);
    hb_shape(font, buf, nullptr, 0);
}

GlyphData &Font::getGlyph(char32_t codePoint, unsigned int size) {
    if (!nextGlyphCache.img) {
        if (!glyphCache.img) {
            glyphCache.createTexture();
        }
        GlyphData *found = glyphCache.getGlyph(this, codePoint, size);
        if (found) {
            return *found;
        }
        nextGlyphCache.createTexture();
    }
    GlyphData *found = nextGlyphCache.getGlyph(this, codePoint, size);
    if (!found) {
        panic("ran out of space in backup texture");
    }
    return *found;
}

GlyphCache::~GlyphCache() {
    if (pixelData) {
        free(pixelData);
    }
}

GlyphData *GlyphCache::getGlyph(Font *font, char32_t codePoint, unsigned int size) {
    {
        // check if we already contain this glyph at this size
        auto it = glyphs.find(GlyphSize(font, codePoint, size));
        if (it != glyphs.end()) {
            return &it->second;
        }
    }
    FT_Face face = (FT_Face)font->ftFace;
    FT_Set_Pixel_Sizes(face, size, size);
    FT_Load_Glyph(face, codePoint, FT_LOAD_DEFAULT);
    FT_GlyphSlot slot = face->glyph;
    int width = slot->bitmap.width;
    int height = slot->bitmap.rows;
    int neededWidth = (width / SIZE_PRECISION + (width % SIZE_PRECISION ? 1 : 0)) * SIZE_PRECISION;
    int x = 0;
    {
        // search columns for space
        for (auto &column : columns) {
            if (CACHE_TEXTURE_SIZE - column.height >= height + PADDING * 2 && column.width >= neededWidth && column.width < neededWidth * 2) {
                // use this one
                auto it = glyphs.emplace(
                    std::piecewise_construct,
                    std::make_tuple(font, codePoint, size),
                    std::make_tuple(ImageRegion(img, IntRectangle(x + PADDING, column.height + PADDING, width, height)), slot->bitmap_left, slot->bitmap_top)
                );
                column.height += height + PADDING * 2;
                pending.emplace_back(font, codePoint, size);
                return &it.first->second;
            }
            x += column.width + PADDING * 2;
        }
        // TODO: for last column, allow expanding column size up to 2x
    }
    {
        // if no space found, create new column
        if (CACHE_TEXTURE_SIZE - x > neededWidth + PADDING * 2) {
            columns.emplace_back(neededWidth, height + PADDING * 2);
            auto it = glyphs.emplace(
                std::piecewise_construct,
                std::make_tuple(font, codePoint, size),
                std::make_tuple(ImageRegion(img, IntRectangle(x + PADDING, PADDING, width, height)), slot->bitmap_left, slot->bitmap_top)
            );
            pending.emplace_back(font, codePoint, size);
            return &it.first->second;
        }
    }
    return nullptr;
}

void GlyphCache::createTexture() {
    GLuint textureId;
    glGenTextures(1, &textureId);
    img = std::make_shared<ImageData>(textureId, IntDimensions(CACHE_TEXTURE_SIZE, CACHE_TEXTURE_SIZE));
    pixelData = (uint8_t*)calloc(CACHE_TEXTURE_SIZE * CACHE_TEXTURE_SIZE, 1);
}

void GlyphCache::commitChanges() {
    if (!pending.empty()) {
        // iterate over pending glyphs
        for (auto &it : pending) {
            Font *font = it.font;
            FT_Face face = (FT_Face)font->ftFace;
            FT_GlyphSlot slot = face->glyph;
            GlyphData &glyphData = glyphs[GlyphSize(it.font, it.glyphIndex, it.size)];
            FT_Set_Pixel_Sizes(face, it.size, it.size);
            FT_Load_Glyph(face, it.glyphIndex, FT_LOAD_RENDER);
            for (unsigned int i = 0; i < slot->bitmap.rows; ++i) {
                memcpy(&pixelData[(glyphData.region.y() + i) * CACHE_TEXTURE_SIZE + glyphData.region.x()], &slot->bitmap.buffer[slot->bitmap.width * i], slot->bitmap.width);
            }
        }
        // upload the texture
        glBindTexture(GL_TEXTURE_2D, img->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, CACHE_TEXTURE_SIZE, CACHE_TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, pixelData);
        // glGenerateMipmap(GL_TEXTURE_2D);
        pending.clear();
    }
}

}
