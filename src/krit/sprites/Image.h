#ifndef KRIT_SPRITES_IMAGE
#define KRIT_SPRITES_IMAGE

#include <string>

#include "krit/Math.h"
#include "krit/Sprite.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Point.h"
#include "krit/math/Rectangle.h"
#include "krit/math/ScaleFactor.h"
#include "krit/render/BlendMode.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Color.h"

namespace krit {
struct RenderContext;

struct Image : public VisibleSprite {
    Point origin;
    Point scroll;
    float angle = 0;
    ImageRegion region;

    Image(const std::string &id);
    Image(std::shared_ptr<ImageData> img) : region(img) {}
    Image(ImageRegion region) : region(region) {}

    int &width() { return this->region.rect.width; }
    int &height() { return this->region.rect.height; }

    void centerOrigin() {
        this->origin.setTo(this->width() / 2.0, this->height() / 2.0);
    }

    Dimensions getSize() override {
        return Dimensions(this->width() * this->scale.x,
                          this->height() * this->scale.y);
    }
    void resize(float w, float h) override {
        this->scale.setTo(w / this->width(), h / this->height());
    }

    void render(RenderContext &ctx) override;
};

}

#endif
