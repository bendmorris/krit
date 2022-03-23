#ifndef KRIT_CAMERA
#define KRIT_CAMERA

#include "krit/math/Dimensions.h"
#include "krit/math/Point.h"
#include "krit/math/ScaleFactor.h"
#include "krit/utils/Signal.h"

namespace krit {

struct Matrix;
struct UpdateContext;

struct CameraTransform {
    Point scroll;
};

struct Camera {
    enum ScaleMode {
        NoScale,
        Stretch,
        KeepWidth,
        KeepHeight,
    };

    Point position;
    Point offset;
    Point anchor;
    Dimensions dimensions;
    ScaleFactor scale;
    ScaleMode scaleMode = NoScale;

    RenderSignal render;

    union {
        struct {
            int min;
            int max;
        } minMax;
    } scaleData;

    float &width() { return this->dimensions.width(); }
    float &height() { return this->dimensions.height(); }

    Camera &center() {
        this->anchor.setTo(0.5, 0.5);
        return *this;
    }

    Camera &setLogicalSize(int width, int height) {
        this->dimensions.setTo(width, height);
        return *this;
    }

    Camera &noScale() {
        this->scaleMode = NoScale;
        return *this;
    }

    Camera &stretch() {
        this->scaleMode = Stretch;
        return *this;
    }

    Camera &keepWidth(int minHeight = 0, int maxHeight = 0);
    Camera &keepHeight(int minWidth = 0, int maxWidth = 0);
    Camera &move(float x, float y);
    Point &transformPoint(Point &p);
    Point &untransformPoint(Point &p);
    Dimensions &scaleDimensions(Dimensions &d);
    Dimensions &unscaleDimensions(Dimensions &d);
    Matrix &transformMatrix(Matrix &m);

    void update(RenderContext &context);
};

}
#endif
