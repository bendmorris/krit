#include "krit/Camera.h"

#include "krit/App.h"
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

void Camera::transformPoint(Point &p) {
    Point position = this->position;
    p += Vec3f(-position.x() + offset.x(), -position.y() + offset.y());
    p += Vec3f(anchor.x() * dimensions.x(), anchor.y() * dimensions.y());
    p *= Vec3f(scale.x(), scale.y(), 1.0);
}

void Camera::untransformPoint(Point &p) {
    Point position = this->position;
    p /= Vec3f(scale.x(), scale.y(), 1.0);
    p -= Vec3f(anchor.x() * dimensions.x(), anchor.y() * dimensions.y());
    p -= Vec3f(-position.x() + offset.x(), -position.y() + offset.y());
}

void Camera::scaleDimensions(Dimensions &d) {
    d.setTo(d.x() * scale.x(), d.y() * scale.y());
}

void Camera::unscaleDimensions(Dimensions &d) {
    d.setTo(d.x() / scale.x(), d.y() / scale.y());
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
            int visibleHeight = height * dimensions.x() / width;
            if (min != 0 && visibleHeight < min) {
                visibleHeight = min;
            } else if (max != 0 && visibleHeight > max) {
                visibleHeight = max;
            }
            scale.setTo(static_cast<float>(width) / dimensions.x(),
                        static_cast<float>(height) / visibleHeight);
            offset.y() = (height / scale.y() - dimensions.y()) / 2;
            break;
        }
        case KeepHeight: {
            int min = scaleData.minMax.min, max = scaleData.minMax.max;
            int visibleWidth = width * dimensions.y() / height;
            if (min != 0 && visibleWidth < min) {
                visibleWidth = min;
            } else if (max != 0 && visibleWidth > max) {
                visibleWidth = max;
            }
            scale.setTo(static_cast<float>(width) / visibleWidth,
                        static_cast<float>(height) / dimensions.y());
            offset.x() = (width / scale.x() - dimensions.x()) / 2;
            break;
        }
    }
}

void Camera::getTransformationMatrix(Matrix4 &m, int width, int height) {
    m.translate(-position.x(), -position.y(), 0);
    if (rotation) {
        m.rotate(rotation);
    }
    if (pitch) {
        m.pitch(pitch);
    }
    m.scale(scale.x(), scale.y(), 1.0);
    m.translate((offset.x() + anchor.x() * dimensions.x()) * scale.x(),
                (offset.y() + anchor.y() * dimensions.y()) * scale.y());

    // screen size
    m.scale(2.0 / width, -2.0 / height, 2.0 / 10000);
    m.translate(-1, 1);

    // perspective
    Matrix4 M;
    M.identity();
    M[11] = 1.0;
    m *= M;
};

void Camera::screenToWorldCoords(Vec3f &screenCoords) {
    auto w = App::ctx.window;
    screenCoords.x() = (screenCoords.x() / w->x()) * 2.0 - 1.0;
    screenCoords.y() = 1.0 - (screenCoords.y() / w->y()) * 2.0;

    // pick a W value and undo the perspective divide
    Vec4f p1{screenCoords.x(), screenCoords.y(), 0.0f, 1.0f};
    Vec4f p2{screenCoords.x() * 2.0f, screenCoords.y() * 2.0f, 1.0f, 2.0f};

    // FIXME: don't need to recompute this each time
    Matrix4 inverseMatrix;
    inverseMatrix.identity();
    getTransformationMatrix(inverseMatrix, w->x(), w->y());
    inverseMatrix.invert();

    p1 = inverseMatrix * p1;
    p2 = inverseMatrix * p2;

    float a = p2.z() / (p2.z() - p1.z());
    float wx = p1.x() * a + p2.x() * (1 - a);
    float wy = p1.y() * a + p2.y() * (1 - a);

    screenCoords.setTo(wx, wy, 0);
};

}
