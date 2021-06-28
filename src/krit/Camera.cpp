#include "krit/Camera.h"

#include "krit/UpdateContext.h"
#include "krit/math/Matrix.h"

namespace krit {

Camera &Camera::keepWidth(int minHeight, int maxHeight) {
    scaleMode = KeepWidth;
    scaleData.minMax.min = minHeight;
    scaleData.minMax.max = maxHeight;
    return *this;
}

Camera &Camera::keepHeight(int minWidth, int maxWidth) {
    scaleMode = KeepHeight;
    scaleData.minMax.min = minWidth;
    scaleData.minMax.max = maxWidth;
    return *this;
}

Camera &Camera::move(float x, float y) {
    this->position.setTo(x, y);
    return *this;
}

Point &Camera::transformPoint(Point &p) {
    Point position = this->position;
    return p
        .add(-position.x + offset.x, -position.y + offset.y)
        .add(anchor.x * dimensions.width(), anchor.y * dimensions.height())
        .multiply(scale.x, scale.y)
    ;
}

Point &Camera::untransformPoint(Point &p) {
    Point position = this->position;
    return p
        .divide(scale.x, scale.y)
        .subtract(anchor.x * dimensions.width(), anchor.y * dimensions.height())
        .subtract(-position.x + offset.x, -position.y + offset.y)
    ;
}

Dimensions &Camera::scaleDimensions(Dimensions &d) {
    return d.setTo(d.width() * scale.x, d.height() * scale.y);
}

Dimensions &Camera::unscaleDimensions(Dimensions &d) {
    return d.setTo(d.width() / scale.x, d.height() / scale.y);
}

Matrix &Camera::transformMatrix(Matrix &m) {
    Point position = this->position;
    return m
        .translate(-position.x + offset.x, -position.y + offset.y)
        .translate(anchor.x * dimensions.width(), anchor.y * dimensions.height())
        .scale(scale.x, scale.y)
    ;
}

void Camera::update(UpdateContext &context) {
    switch (scaleMode) {
        case NoScale: {
            // nothing to do
            break;
        }
        case Stretch: {
            // stretch to show the camera's logical size
            scale.setTo(
                context.window->width() / dimensions.width(),
                context.window->height() / dimensions.height()
            );
            break;
        }
        case KeepWidth: {
            int min = scaleData.minMax.min,
                max = scaleData.minMax.max;
            int visibleHeight = context.window->height() * dimensions.width() / context.window->width();
            if (visibleHeight < min) {
                visibleHeight = min;
            } else if (visibleHeight > max) {
                visibleHeight = max;
            }
            scale.setTo(
                static_cast<float>(context.window->width()) / dimensions.width(),
                static_cast<float>(context.window->height()) / visibleHeight
            );
            offset.y = (context.window->height() / scale.y - dimensions.height()) / 2;
            break;
        }
        case KeepHeight: {
            int min = scaleData.minMax.min,
                max = scaleData.minMax.max;
            int visibleWidth = context.window->width() * dimensions.height() / context.window->height();
            if (visibleWidth < min) {
                visibleWidth = min;
            } else if (visibleWidth > max) {
                visibleWidth = max;
            }
            scale.setTo(
                static_cast<float>(context.window->width()) / visibleWidth,
                static_cast<float>(context.window->height()) / dimensions.height()
            );
            offset.x = (context.window->width() / scale.x - dimensions.width()) / 2;
            break;
        }
    }
}

}
