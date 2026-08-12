#ifndef KRIT_SPRITES_NINESLICE
#define KRIT_SPRITES_NINESLICE

#include <memory>

#include "krit/Engine.h"
#include "krit/Sprite.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Point.h"
#include "krit/render/ImageRegion.h"

namespace krit {
struct RenderContext;

struct NineSlice : public Sprite {
    Point origin{0};
    float pitch = 0;

    ImageRegion base;

    int leftWidth{0};
    int rightWidth{0};
    int topHeight{0};
    int bottomHeight{0};
    bool debugME { false };

    float borderScale{1};

    void setSrc(const ImageRegion &img) { base = img; }
    void setBorder(int b) {
        leftWidth = rightWidth = topHeight = bottomHeight = b;
    }
    void setBorderWidth(int w) { leftWidth = rightWidth = w; }
    void setBorderHeight(int h) { topHeight = bottomHeight = h; }

    NineSlice() {}

    NineSlice(const ImageRegion &base, int border)
        : base(base), leftWidth(border), rightWidth(border), topHeight(border),
          bottomHeight(border) {}

    NineSlice(const ImageRegion &base, int width, int height)
        : base(base), leftWidth(width), rightWidth(width), topHeight(height),
          bottomHeight(height) {}

    NineSlice(const ImageRegion &base, int leftWidth, int rightWidth, int topHeight,
              int bottomHeight)
        : base(base), leftWidth(leftWidth), rightWidth(rightWidth),
          topHeight(topHeight), bottomHeight(bottomHeight) {}

    void centerOrigin() {
        this->origin.setTo(this->base.width() / 2.0, this->base.height() / 2.0);
    }

    void render(RenderContext &ctx) override;
};

}

#endif
