#ifndef KRIT_SPRITES_IMAGE
#define KRIT_SPRITES_IMAGE

#include "krit/Math.h"
#include "krit/Sprite.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Point.h"
#include "krit/math/Rectangle.h"
#include "krit/render/BlendMode.h"
#include "krit/render/ImageRegion.h"
#include "krit/utils/Color.h"
#include <optional>
#include <string>

namespace krit {
struct RenderContext;

struct Image : public Sprite {
    Point origin;
    Point scroll;
    float angle = 0;
    float pitch = 0;
    ImageRegion region;

    Image();
    Image(const std::string &id);
    Image(std::shared_ptr<ImageData> img);
    Image(ImageRegion region);

    void setSrc(const ImageRegion &id);
    void setScale(float sx, std::optional<float> sy = {}) {
        setScaleX(sx);
        setScaleY(sy.value_or(sx));
    }
    float getScaleX() { return dimensions.x / region.rect.width; }
    void setScaleX(float s) { dimensions.x = region.rect.width * s; }
    float getScaleY() { return dimensions.y / region.rect.height; }
    void setScaleY(float s) { dimensions.y = region.rect.height * s; }

    void centerOrigin() {
        this->origin.setTo(this->region.rect.width / 2.0,
                           this->region.rect.height / 2.0);
    }

    void render(RenderContext &ctx) override;
};

}

#endif
