#ifndef KRIT_SPRITES_IMAGE
#define KRIT_SPRITES_IMAGE

#include "krit/render/BlendMode.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Color.h"
#include "krit/Sprite.h"
#include "krit/Math.h"

namespace krit {

struct Image: public VisibleSprite {
    Point origin;
    Point scroll;
    float angle = 0;
    ImageRegion region;

    Image(const std::string &id);
    Image(ImageRegion region): region(region) {}

    int &width() { return this->region.rect.width; }
    int &height() { return this->region.rect.height; }

    void centerOrigin() {
        this->origin.setTo(this->width() / 2.0, this->height() / 2.0);
    }

    Dimensions getSize() override { return Dimensions(this->width() * this->scale.x, this->height() * this->scale.y); }
    void resize(double w, double h) override { this->scale.setTo(w / this->width(), h / this->height()); }

    void render(RenderContext &ctx) override;
};

}

#endif
