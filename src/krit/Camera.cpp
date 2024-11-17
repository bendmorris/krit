#include "krit/Camera.h"

#include "krit/Engine.h"
#include "krit/Window.h"
#include "krit/math/Matrix.h"
#include "krit/render/FrameBuffer.h"
#include "krit/render/RenderContext.h"

namespace krit {

Camera::~Camera() {
    if (fb) {
        delete fb;
        fb = nullptr;
    }
}

Camera &Camera::move(float x, float y) {
    this->position.setTo(x, y, this->position.z());
    return *this;
}

void Camera::transformPoint(Point &p) {
    Point position = this->position;
    p += Vec3f(-position.x(), -position.y(), -position.z());
    p += Vec3f(anchor.x() * dimensions.x(), anchor.y() * dimensions.y());
    p *= Vec3f(scale.x(), scale.y(), 1.0);
}

void Camera::untransformPoint(Point &p) {
    Point position = this->position;
    p /= Vec3f(scale.x(), scale.y(), 1.0);
    p -= Vec3f(anchor.x() * dimensions.x(), anchor.y() * dimensions.y());
    p -= Vec3f(-position.x(), -position.y(), -position.z());
}

void Camera::scaleDimensions(Dimensions &d) {
    d.setTo(d.x() * scale.x(), d.y() * scale.y());
}

void Camera::unscaleDimensions(Dimensions &d) {
    d.setTo(d.x() / scale.x(), d.y() / scale.y());
}

void Camera::update() {
    double width = engine->window.x(), height = engine->window.y();
    double ratio = width / height;
    if (ratio < minRatio) {
        // too narrow; top and bottom letterboxing
        ratio = minRatio;
        offset.x() = 0;
        offset.y() = (height - (width / minRatio)) / 2;
        float s = width / dimensions.x();
        scale.setTo(s, s);
    } else {
        if (ratio > maxRatio) {
            // too wide; left and right letterboxing
            ratio = maxRatio;
            offset.x() = (width - (height * maxRatio)) / 2;
            offset.y() = 0;
        } else {
            offset.setTo(0, 0);
        }
        float s = height / dimensions.y();
        scale.setTo(s, s);
    }
    currentDimensions.setTo(ratio * dimensions.y(), dimensions.y());
}

void Camera::getTransformationMatrix(Matrix4 &m, int width, int height) {
    m.translate(-position.x(), -position.y(), 0);
    if (rotation) {
        m.rotate(rotation);
    }
    if (pitch) {
        m.pitch(pitch);
    }
    if (roll) {
        m.roll(roll);
    }
    m.translate(0, 0, -position.z());
    m.scale(scale.x(), scale.y(), 1.0);
    m.translate((anchor.x() * currentDimensions.x()) * scale.x(),
                (anchor.y() * currentDimensions.y()) * scale.y());

    // screen size
    m.scale(2.0 / width, -2.0 / height, 1.0 / 2000);
    m.translate(-1, 1);

    // perspective
    Matrix4 M;
    M.identity();
    M[11] = 1.0;
    m *= M;
};

void Camera::worldToScreenCoords(Vec3f &worldCoords) {
    Matrix4 matrix;
    matrix.identity();
    getTransformationMatrix(matrix, viewportWidth(), viewportHeight());

    Vec4f result(worldCoords.x(), worldCoords.y(), worldCoords.z(), 1.0f);
    result = matrix * result;
    result.x() /= result.w();
    result.y() /= result.w();
    result.z() /= result.w();
    result.w() /= result.w();

    result.x() =
        ((result.x() + 1.0) / 2.0) * static_cast<double>(viewportWidth());
    result.y() =
        ((1.0 - result.y()) / 2.0) * static_cast<double>(viewportHeight());
    result.x() += offset.x();
    result.y() += offset.y();

    worldCoords.setTo(result.x(), result.y(), result.z());
};

void Camera::screenToWorldCoords(Vec3f &screenCoords) {
    // printf("%i, %i, %i\n", offset.x(), viewportWidth(), w.x());
    screenCoords.x() -= offset.x();
    screenCoords.y() -= offset.y();
    screenCoords.x() =
        (screenCoords.x() / static_cast<double>(viewportWidth())) * 2.0 - 1.0;
    screenCoords.y() =
        1.0 - (screenCoords.y() / static_cast<double>(viewportHeight())) * 2.0;

    // pick a W value and undo the perspective divide
    Vec4f p1{screenCoords.x(), screenCoords.y(), 0.0f, 1.0f};
    Vec4f p2{screenCoords.x() * 2.0f, screenCoords.y() * 2.0f, 1.0f, 2.0f};

    // FIXME: don't need to recompute this each time
    Matrix4 inverseMatrix;
    inverseMatrix.identity();
    getTransformationMatrix(inverseMatrix, viewportWidth(), viewportHeight());
    inverseMatrix.invert();

    p1 = inverseMatrix * p1;
    p2 = inverseMatrix * p2;

    float a = p2.z() / (p2.z() - p1.z());
    float wx = p1.x() * a + p2.x() * (1 - a);
    float wy = p1.y() * a + p2.y() * (1 - a);

    screenCoords.setTo(wx, wy, 0);
};

}
