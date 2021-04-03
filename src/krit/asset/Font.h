#ifndef KRIT_ASSET_FONT
#define KRIT_ASSET_FONT

#include "krit/math/Point.h"
#include "krit/math/ScaleFactor.h"
#include "krit/math/Rectangle.h"
#include "krit/render/ImageData.h"
#include "krit/utils/Color.h"
#include <memory>

namespace krit {

struct GlyphData {
    int page = 0;
    int id = 0;
    IntRectangle rect;
    IntPoint offset;
    int xAdvance = 0;

    GlyphData() {}
};

struct Font {
    int size = 0;
    int lineHeight = 0;

    virtual GlyphData getGlyph(int c) = 0;
    virtual std::shared_ptr<ImageData> &getPage(int i) = 0;
    virtual double kern(int32_t lastChar, int32_t thisChar) = 0;

    virtual ~Font() {}
};

}

#endif