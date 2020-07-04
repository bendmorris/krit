#include "krit/Camera.h"

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

Camera &Camera::move(double x, double y) {
    this->position.setTo(x, y);
    return *this;
}

Point &Camera::transformPoint(CameraTransform *transform, Point &p) {
    Point position = this->position;
    if (transform) {
        position.multiply(transform->scroll.x, transform->scroll.y);
    }
    return p
        .add(-position.x + offset.x, -position.y + offset.y)
        .add(anchor.x * dimensions.width(), anchor.y * dimensions.height())
        .multiply(scale.x, scale.y)
    ;
}

Point &Camera::untransformPoint(CameraTransform *transform, Point &p) {
    Point position = this->position;
    if (transform) {
        position.multiply(transform->scroll.x, transform->scroll.y);
    }
    return p
        .subtract(anchor.x * dimensions.width(), anchor.y * dimensions.height())
        .subtract(-position.x + offset.x, -position.y + offset.y)
        .divide(scale.x, scale.y)
    ;
}

Dimensions &Camera::scaleDimensions(CameraTransform *transform, Dimensions &d) {
    return d.setTo(d.width() * scale.x, d.height() * scale.y);
}

Dimensions &Camera::unscaleDimensions(CameraTransform *transform, Dimensions &d) {
    return d.setTo(d.width() / scale.x, d.height() / scale.y);
}

Matrix &Camera::transformMatrix(CameraTransform *transform, Matrix &m) {
    Point position = this->position;
    if (transform) {
        position.multiply(transform->scroll.x, transform->scroll.y);
    }
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
                static_cast<double>(context.window->width()) / dimensions.width(),
                static_cast<double>(context.window->height()) / visibleHeight
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
                static_cast<double>(context.window->width()) / visibleWidth,
                static_cast<double>(context.window->height()) / dimensions.height()
            );
            offset.x = (context.window->width() / scale.x - dimensions.width()) / 2;
            break;
        }
    }
}

}
