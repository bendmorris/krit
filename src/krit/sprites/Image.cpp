#include <memory>

#include "krit/Engine.h"
#include "krit/math/Matrix.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"
#include "krit/sprites/Image.h"
#include "krit/utils/Color.h"

namespace krit {

Image::Image() {}

Image::Image(ImageRegion region) : region(region) {
    dimensions.setTo(region.rect.width, region.rect.height);
}

Image::Image(const std::string &id) : Image(engine->getImage(id)) {}
Image::Image(std::shared_ptr<ImageData> img) : Image(ImageRegion(img)) {}

void Image::setSrc(const ImageRegion &region) {
    this->region = region;
    dimensions.setTo(region.rect.width, region.rect.height);
}

void Image::render(RenderContext &ctx) {
    if (!region.img || (this->color.a <= 0 && !shader)) {
        return;
    }
    // ctx.transform = (struct RenderTransform) {scroll: this->scroll};
    Matrix4 matrix;
    matrix.identity();
    matrix.translate(-this->origin.x, -this->origin.y);
    matrix.scale(this->dimensions.x / region.rect.width,
                 this->dimensions.y / region.rect.height);
    if (this->angle) {
        matrix.rotate(this->angle);
    }
    if (this->pitch) {
        matrix.pitch(this->pitch);
    }
    matrix.translate(this->position.x, this->position.y, this->position.z);
    DrawKey key;
    key.shader = this->shader;
    key.image = this->region.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;
    ctx.addRect(key, this->region.rect, matrix, this->color, zIndex);
}

}
