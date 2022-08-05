#ifndef KRIT_SPRITES_BACKDROP
#define KRIT_SPRITES_BACKDROP

#include <string>

#include "krit/Math.h"
#include "krit/Sprite.h"
#include "krit/math/Point.h"
#include "krit/math/Rectangle.h"
#include "krit/render/BlendMode.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Color.h"

namespace krit {
struct RenderContext;

struct Backdrop : public VisibleSprite {
    static Backdrop *create(const std::string &id) { return new Backdrop(id); }

    float angle = 0;
    ImageRegion region;
    Point scroll;
    bool repeatX = true;
    bool repeatY = true;

    Backdrop(const std::string &id);
    Backdrop(ImageRegion region) : region(region) {}

    int &width() { return this->region.rect.width; }
    int &height() { return this->region.rect.height; }

    void resize(float w, float h) override {
        this->scale.setTo(w / this->width(), h / this->height());
    }

    void render(RenderContext &ctx) override;
};

}

#endif
