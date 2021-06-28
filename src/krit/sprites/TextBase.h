#ifndef KRIT_SPRITES_TEXTBASE
#define KRIT_SPRITES_TEXTBASE

#include "krit/math/Dimensions.h"
#include "krit/math/ScaleFactor.h"
#include "krit/utils/Color.h"

namespace krit {

enum AlignType {
    LeftAlign,
    CenterAlign,
    RightAlign,
};

struct NewlineData {
    Dimensions first;
    AlignType second;

    NewlineData(const Dimensions &d, AlignType a) : first(d), second(a) {}
};

struct GlyphRenderData {
    int32_t c = 0;
    Color color;
    ScaleFactor scale;
    Point position;

    GlyphRenderData() {}
    GlyphRenderData(const Point &position) : position(position) {}
    GlyphRenderData(int32_t c, Color color, ScaleFactor &scale,
                    const Point &position)
        : c(c), color(color), scale(scale), position(position) {}
};

}

#endif
