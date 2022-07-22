#include <memory>

#include "krit/App.h"
#include "krit/Engine.h"
#include "krit/math/Matrix.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"
#include "krit/sprites/Image.h"
#include "krit/utils/Color.h"

namespace krit {

Image::Image(std::shared_ptr<ImageData> img) : region(img) {
    dimensions.setTo(region.rect.width, region.rect.height);
}
Image::Image(ImageRegion region) : region(region) {
    dimensions.setTo(region.rect.width, region.rect.height);
}

Image::Image(const std::string &id) : region(App::ctx.engine->getImage(id)) {
    dimensions.setTo(region.rect.width, region.rect.height);
}

void Image::update(UpdateContext &ctx) {
    dimensions.setTo(region.rect.width * abs(scale.x()),
                     region.rect.height * abs(scale.y()));
}

void Image::render(RenderContext &ctx) {
    if (this->color.a <= 0 && !shader) {
        return;
    }
    // ctx.transform = (struct RenderTransform) {scroll: this->scroll};
    Matrix4 matrix;
    matrix.identity();
    matrix.translate(-this->origin.x(), -this->origin.y());
    if (this->angle) {
        matrix.rotate(this->angle);
    }
    if (this->pitch) {
        matrix.pitch(this->pitch);
    }
    matrix.scale(this->scale.x(), this->scale.y());
    matrix.translate(this->position.x(), this->position.y());
    DrawKey key;
    key.shader = this->shader;
    key.image = this->region.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;
    ctx.addRect(key, this->region.rect, matrix, this->color, zIndex);
}

}
