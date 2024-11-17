#ifndef KRIT_CAMERA
#define KRIT_CAMERA

#include "krit/math/Dimensions.h"
#include "krit/math/Matrix.h"
#include "krit/math/Point.h"
#include "krit/utils/Signal.h"

namespace krit {

struct FrameBuffer;
struct Matrix;
struct UpdateContext;

struct Camera {
    Point position;
    IntPoint offset;
    Point anchor;
    Dimensions dimensions;
    Dimensions currentDimensions;
    Vec2f scale{1, 1};
    double minRatio = 16.0 / 9;
    double maxRatio = 16.0 / 9;
    FrameBuffer *fb = nullptr;

    Signal render;

    float &width() { return this->dimensions.x(); }
    float &height() { return this->dimensions.y(); }

    int viewportWidth() {
        return round(this->currentDimensions.x() * scale.x());
    }
    int viewportHeight() {
        return round(this->currentDimensions.y() * scale.y());
    }

    float rotation = 0;
    float pitch = 0;
    float roll = 0;

    Camera() : dimensions(3840, 2160), currentDimensions(3840, 2160) {}
    virtual ~Camera();

    Camera &center() {
        this->anchor.setTo(0.5, 0.5);
        return *this;
    }

    Camera &setLogicalSize(int width, int height) {
        this->dimensions.setTo(width, height);
        return *this;
    }

    Camera &move(float x, float y);
    /**
     * Apply the Camera's transformation to a given point in camera space,
     * converting it to screen coordinates.
     */
    void transformPoint(Point &p);
    /**
     * Apply the inverse of the Camera's transformation to a given point in
     * screen space, converting it to camera-local coordinates.
     */
    void untransformPoint(Point &p);
    void scaleDimensions(Dimensions &d);
    void unscaleDimensions(Dimensions &d);

    void getTransformationMatrix(Matrix4 &m, int width, int height);

    void update();

    void worldToScreenCoords(Vec3f &screenCoords);
    void screenToWorldCoords(Vec3f &screenCoords);

    void resetRotation() { rotation = pitch = roll = 0; }
};

}
#endif
