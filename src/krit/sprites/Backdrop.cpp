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
    scaledDimensions *= this->scale;
    Point pos = this->position;
    int xi = 1, yi = 1;
    if (this->repeatX) {
        pos.x() = fmod(pos.x(), scaledDimensions.x());
        if (pos.x() > 0) {
            pos.x() -= scaledDimensions.x();
        }
        xi = static_cast<int>(ceil(ctx.window->x() - pos.x()) /
                                  scaledDimensions.x() +
                              1);
    }
    if (this->repeatY) {
        pos.y() = fmod(pos.y(), scaledDimensions.y());
        if (pos.y() > 0) {
            pos.y() -= scaledDimensions.y();
        }
        yi = static_cast<int>(ceil(ctx.window->y() - pos.y()) /
                                  scaledDimensions.y() +
                              1);
    }
    DrawKey key;
    key.image = this->region.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;
    key.shader = this->shader;
    Matrix4 m;
    m.identity();
    m.a() = static_cast<float>(scaledDimensions.x()) / this->width();
    m.d() = static_cast<float>(scaledDimensions.y()) / this->height();
    for (int y = 0; y < yi; ++y) {
        for (int x = 0; x < xi; ++x) {
            m.tx() = pos.x() + scaledDimensions.x() * x;
            m.ty() = pos.y() + scaledDimensions.y() * y;
            ctx.addRect(key, this->region.rect, m, this->color);
        }
    }

    // ctx.transform = oldTransform;
}

}
