#include "krit/sprites/Backdrop.h"

#include <math.h>
#include <memory>

#include "krit/App.h"
#include "krit/Engine.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Matrix.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Color.h"

namespace krit {

Backdrop::Backdrop(const std::string &id)
    : region(App::ctx.engine->getImage(id)) {}

void Backdrop::render(RenderContext &ctx) {
    if (this->color.a <= 0 && !shader) {
        return;
    }
    // FIXME: scroll...
    // CameraTransform transform;
    // transform.scroll = scroll;
    // CameraTransform *oldTransform = ctx.transform;
    // ctx.transform = &transform;

    Dimensions scaledDimensions(this->width(), this->height());
    ctx.transformDimensions(
        scaledDimensions.multiply(this->scale.x, this->scale.y));
    Point scaledPosition = this->position;
    ctx.transformPoint(scaledPosition);
    int xi = 1, yi = 1;
    if (this->repeatX) {
        scaledPosition.x = fmod(scaledPosition.x, scaledDimensions.width());
        if (scaledPosition.x > 0) {
            scaledPosition.x -= scaledDimensions.width();
        }
        xi = static_cast<int>(ceil(ctx.window->width() - scaledPosition.x) /
                                  scaledDimensions.width() +
                              1);
    }
    if (this->repeatY) {
        scaledPosition.y = fmod(scaledPosition.y, scaledDimensions.height());
        if (scaledPosition.y > 0) {
            scaledPosition.y -= scaledDimensions.height();
        }
        yi = static_cast<int>(ceil(ctx.window->height() - scaledPosition.y) /
                                  scaledDimensions.height() +
                              1);
    }
    DrawKey key;
    key.image = this->region.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;
    key.shader = this->shader;
    Matrix m(static_cast<float>(scaledDimensions.width()) / this->width(), 0, 0,
             static_cast<float>(scaledDimensions.height()) / this->height(), 0,
             0);
    for (int y = 0; y < yi; ++y) {
        for (int x = 0; x < xi; ++x) {
            m.tx = scaledPosition.x + scaledDimensions.width() * x;
            m.ty = scaledPosition.y + scaledDimensions.height() * y;
            ctx.addRectRaw(key, this->region.rect, m, this->color);
        }
    }

    // ctx.transform = oldTransform;
}

}
