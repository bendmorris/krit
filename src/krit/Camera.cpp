#include "krit/Camera.h"

#include "krit/Window.h"
#include "krit/math/Matrix.h"
#include "krit/render/RenderContext.h"

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
    return p.add(-position.x + offset.x, -position.y + offset.y)
        .add(anchor.x * dimensions.width(), anchor.y * dimensions.height())
        .multiply(scale.x, scale.y);
}

Point &Camera::untransformPoint(Point &p) {
    Point position = this->position;
    return p.divide(scale.x, scale.y)
        .subtract(anchor.x * dimensions.width(), anchor.y * dimensions.height())
        .subtract(-position.x + offset.x, -position.y + offset.y);
}

Dimensions &Camera::scaleDimensions(Dimensions &d) {
    return d.setTo(d.width() * scale.x, d.height() * scale.y);
}

Dimensions &Camera::unscaleDimensions(Dimensions &d) {
    return d.setTo(d.width() / scale.x, d.height() / scale.y);
}

Matrix &Camera::transformMatrix(Matrix &m) {
    Point position = this->position;
    return m.translate(-position.x + offset.x, -position.y + offset.y)
        .translate(anchor.x * dimensions.width(),
                   anchor.y * dimensions.height())
        .scale(scale.x, scale.y);
}

void Camera::update(RenderContext &context) {
    float width = context.width(), height = context.height();
    switch (scaleMode) {
        case NoScale: {
            // nothing to do
            break;
        }
        case Stretch: {
            // stretch to show the camera's logical size
            scale.setTo(1, 1);
            break;
        }
        case KeepWidth: {
            int min = scaleData.minMax.min, max = scaleData.minMax.max;
            int visibleHeight = height * dimensions.width() / width;
            if (min != 0 && visibleHeight < min) {
                visibleHeight = min;
            } else if (max != 0 && visibleHeight > max) {
                visibleHeight = max;
            }
            scale.setTo(static_cast<float>(width) / dimensions.width(),
                        static_cast<float>(height) / visibleHeight);
            offset.y = (height / scale.y - dimensions.height()) / 2;
            break;
        }
        case KeepHeight: {
            int min = scaleData.minMax.min, max = scaleData.minMax.max;
            int visibleWidth = width * dimensions.height() / height;
            if (min != 0 && visibleWidth < min) {
                visibleWidth = min;
            } else if (max != 0 && visibleWidth > max) {
                visibleWidth = max;
            }
            scale.setTo(static_cast<float>(width) / visibleWidth,
                        static_cast<float>(height) / dimensions.height());
            offset.x = (width / scale.x - dimensions.width()) / 2;
            break;
        }
    }
}

}
