#include "krit/asset/Font.h"
#include "harfbuzz/hb.h"
#include "krit/Engine.h"
#include "krit/io/Io.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Rectangle.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include "krit/utils/Panic.h"
#include <cstring>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TYPES_H
#include FT_STROKER_H
#include <stdlib.h>
#include <tuple>
#include <utility>

namespace krit {

#ifndef __EMSCRIPTEN__
static const int BYTES_PER_PIXEL = 1;
#else
static const int BYTES_PER_PIXEL = 4;
#endif

static FT_Stroker stroker;

template <>
std::shared_ptr<Font> AssetLoader<Font>::loadAsset(const std::string &path) {
    std::string content = engine->io->readFile(path);
    // FIXME: do we leak content here, or does FT free it?
    return std::make_shared<Font>(path, content);
}

template <> AssetType AssetLoader<Font>::type() { return FontAsset; }

template <> size_t AssetLoader<Font>::cost(Font *f) { return sizeof(Font); }

static FT_Library ftLibrary;

FontManager::FontManager() {
    int error = FT_Init_FreeType(&ftLibrary);
    if (error) {
        panic("failed to initialize freetype: %i\n", error);
    }
    FT_Stroker_New(ftLibrary, &stroker);
}

FontManager::~FontManager() {
    FT_Done_FreeType(ftLibrary);
    ftLibrary = nullptr;
}

void FontManager::commit() {
    if (glyphCache.img) {
        glyphCache.commitChanges();
    }
    if (nextGlyphCache.img) {
        nextGlyphCache.commitChanges();
    }
}

void FontManager::flush() {
    if (nextGlyphCache.img) {
        glyphCache = std::move(nextGlyphCache);
        nextGlyphCache = GlyphCache();
    }
}

std::shared_ptr<Font> FontManager::registerFont(const std::string &name,
                                                const std::string &path) {
    auto font = AssetLoader<Font>::loadAsset(path);
    fontRegistry.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                         std::forward_as_tuple(font));
    return font;
}

GlyphData &FontManager::getGlyph(Font *font, char32_t codePoint,
                                 unsigned int size, unsigned int border) {
    if (!nextGlyphCache.img) {
        if (!glyphCache.img) {
            glyphCache.createTexture();
        }
        GlyphData *found = glyphCache.getGlyph(font, codePoint, size, border);
        if (found) {
            return *found;
        }
        nextGlyphCache.createTexture();
    }
    GlyphData *found = nextGlyphCache.getGlyph(font, codePoint, size, border);
    if (!found) {
        panic("ran out of space in backup texture");
    }
    return *found;
}

Font::Font(const std::string &path, const std::string &fontData) : path(path) {
    // harfbuzz face initialization
    this->fontData = fontData;
    blob = hb_blob_create(this->fontData.c_str(), this->fontData.size(),
                          HB_MEMORY_MODE_READONLY, nullptr, nullptr);
    face = hb_face_create(blob, 0);
    font = hb_font_create(face);
    hb_font_set_scale(font, 64, 64);
    // freetype face initialization
    int error =
        FT_New_Memory_Face(ftLibrary, (const FT_Byte *)this->fontData.c_str(),
                           this->fontData.size(), 0, (FT_Face *)&ftFace);
    if (error) {
        panic("failed to initialize font: %s", path.c_str());
    }
}

Font::~Font() {
    // if (ftFace) {
    //     FT_Done_Face((FT_Face)ftFace);
    // }
    if (font) {
        hb_font_destroy(font);
    }
    if (face) {
        hb_face_destroy(face);
    }
    if (blob) {
        hb_blob_destroy(blob);
    }
}

void Font::shape(hb_buffer_t *buf) {
    hb_feature_t userfeatures[1];
    userfeatures[0].tag = HB_TAG('l', 'i', 'g', 'a');
    userfeatures[0].value = ligatures ? 1 : 0;
    userfeatures[0].start = HB_FEATURE_GLOBAL_START;
    userfeatures[0].end = HB_FEATURE_GLOBAL_END;
    hb_shape(font, buf, userfeatures, 1);
}

GlyphData *GlyphCache::getGlyph(Font *font, char32_t codePoint,
                                unsigned int size, unsigned int border) {
    {
        // check if we already contain this glyph at this size
        auto it = glyphs.find(GlyphSize(font, codePoint, size, border));
        if (it != glyphs.end()) {
            return &it->second;
        }
    }
    FT_Face face = (FT_Face)font->ftFace;
    FT_Set_Pixel_Sizes(face, size, size);
    FT_Load_Glyph(face, codePoint, FT_LOAD_DEFAULT);
    FT_Glyph glyph;
    FT_Get_Glyph(face->glyph, &glyph);
    if (border > 0) {
        FT_Stroker_Set(stroker, border * 64, FT_STROKER_LINECAP_ROUND,
                       FT_STROKER_LINEJOIN_ROUND, 0);
        FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
    }
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    FT_BitmapGlyph bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);

    int width = bitmap->bitmap.width;
    int height = bitmap->bitmap.rows;
    int neededWidth =
        (width / SIZE_PRECISION + (width % SIZE_PRECISION ? 1 : 0)) *
        SIZE_PRECISION;
    int x = 0;
    {
        // search columns for space
        for (auto &column : columns) {
            if (CACHE_TEXTURE_SIZE - column.height >= height + PADDING * 2 &&
                column.width >= neededWidth && column.width < neededWidth * 2) {
                // use this one
                auto it = glyphs.emplace(
                    std::piecewise_construct,
                    std::make_tuple(font, codePoint, size, border),
                    std::make_tuple(
                        ImageRegion(img, IntRectangle(x + PADDING,
                                                      column.height + PADDING,
                                                      width, height)),
                        bitmap->left, bitmap->top));
                column.height += height + PADDING * 2;
                pending.emplace_back(font, codePoint, size, border);
                FT_Done_Glyph(glyph);
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
                std::make_tuple(font, codePoint, size, border),
                std::make_tuple(
                    ImageRegion(
                        img, IntRectangle(x + PADDING, PADDING, width, height)),
                    bitmap->left, bitmap->top));
            pending.emplace_back(font, codePoint, size, border);
            FT_Done_Glyph(glyph);
            return &it.first->second;
        }
    }
    FT_Done_Glyph(glyph);
    return nullptr;
}

void GlyphCache::createTexture() {
    GLuint textureId;
    glGenTextures(1, &textureId);
    if (!textureId) {
        LOG_ERROR("failed to generate texture for GlyphCache");
    }
    img = std::make_shared<ImageData>(
        textureId, IntDimensions(CACHE_TEXTURE_SIZE, CACHE_TEXTURE_SIZE));
    pixelData = std::make_unique<uint8_t[]>(
        CACHE_TEXTURE_SIZE * CACHE_TEXTURE_SIZE * BYTES_PER_PIXEL);
}

void GlyphCache::commitChanges() {
    if (!pending.empty()) {
        // iterate over pending glyphs
        for (auto &it : pending) {
            Font *font = it.font;
            FT_Face face = (FT_Face)font->ftFace;
            GlyphData &glyphData =
                glyphs[GlyphSize(it.font, it.glyphIndex, it.size, it.border)];
            FT_Set_Pixel_Sizes(face, it.size, it.size);
            FT_Load_Glyph(face, it.glyphIndex, FT_LOAD_DEFAULT);
            FT_Glyph glyph;
            FT_Get_Glyph(face->glyph, &glyph);
            if (it.border > 0) {
                FT_Stroker_Set(stroker, it.border * 64,
                               FT_STROKER_LINECAP_ROUND,
                               FT_STROKER_LINEJOIN_ROUND, 0);
                FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
            }
            FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_LIGHT, nullptr, true);
            FT_BitmapGlyph bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);
            for (unsigned int i = 0; i < bitmap->bitmap.rows; ++i) {
#ifndef __EMSCRIPTEN__
                memcpy(
                    &pixelData[(glyphData.region.y() + i) * CACHE_TEXTURE_SIZE +
                               glyphData.region.x()],
                    &bitmap->bitmap.buffer[bitmap->bitmap.pitch * i],
                    bitmap->bitmap.width);
#else
                int offset = (glyphData.region.y() + i) * CACHE_TEXTURE_SIZE +
                             glyphData.region.x();
                for (int j = 0; j < bitmap->bitmap.width; ++j) {
                    pixelData[(offset + j) * BYTES_PER_PIXEL + 0] =

                        bitmap->bitmap.buffer[bitmap->bitmap.pitch * i + j];
                    pixelData[(offset + j) * BYTES_PER_PIXEL + 1] =
                        pixelData[(offset + j) * BYTES_PER_PIXEL + 2] = 0;
                    pixelData[(offset + j) * BYTES_PER_PIXEL + 3] = 0xff;
                }
#endif
            }
            FT_Done_Glyph(glyph);
        }
        // upload the texture
        glBindTexture(GL_TEXTURE_2D, img->texture);
#ifndef __EMSCRIPTEN__
        auto mode = GL_RED;
#else
        auto mode = GL_RGBA;
#endif
        glTexImage2D(GL_TEXTURE_2D, 0, mode, CACHE_TEXTURE_SIZE,
                     CACHE_TEXTURE_SIZE, 0, mode, GL_UNSIGNED_BYTE,
                     pixelData.get());
        glBindTexture(GL_TEXTURE_2D, 0);
        pending.clear();
    }
}

template <> bool AssetLoader<Font>::assetIsReady(Font *img) { return true; }

}
