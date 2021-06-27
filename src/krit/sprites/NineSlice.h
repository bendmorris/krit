#ifndef KRIT_SPRITES_NINESLICE
#define KRIT_SPRITES_NINESLICE

#include "krit/Sprite.h"
#include "krit/render/ImageRegion.h"
#include <memory>

namespace krit {

struct NineSlice : public VisibleSprite {
    Point origin;

    ImageRegion ul;
    ImageRegion uc;
    ImageRegion ur;
    ImageRegion cl;
    ImageRegion cc;
    ImageRegion cr;
    ImageRegion bl;
    ImageRegion bc;
    ImageRegion br;

    bool scaleBorder = false;

    NineSlice() {}

    NineSlice(ImageRegion &base, int border)
        : NineSlice(base, border, border) {}

    NineSlice(ImageRegion &base, int width, int height)
        : NineSlice(base, width, width, height, height) {}

    NineSlice(ImageRegion &base, int leftWidth, int rightWidth, int topHeight,
              int bottomHeight);

    void centerOrigin() {
        this->origin.setTo(this->width() / 2.0, this->height() / 2.0);
    }

    void resize(float w, float h) override {
        this->dimensions.setTo(w / this->scale.x, h / this->scale.y);
    }

    void render(RenderContext &ctx) override;
};

}

#endif
