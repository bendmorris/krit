#include "krit/App.h"
#include "krit/Camera.h"
#include "krit/sprites/Image.h"

namespace krit {

Image::Image(const std::string &id): region(App::ctx.engine->getImage(id)) {}

void Image::render(RenderContext &ctx) {
    if (this->color.a <= 0) {
        return;
    }
    // ctx.transform = (struct RenderTransform) {scroll: this->scroll};
    Matrix matrix(1, 0, 0, 1, -this->origin.x, -this->origin.y);
    matrix
        .rotate(this->angle)
        .scale(this->scale.x, this->scale.y)
        .translate(this->position.x, this->position.y);
    DrawKey key;
    key.shader = this->shader;
    key.image = this->region.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;
    ctx.addRect(key, this->region.rect, matrix, this->color);
}

}
