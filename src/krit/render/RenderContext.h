#ifndef KRIT_RENDER_RENDER_CONTEXT
#define KRIT_RENDER_RENDER_CONTEXT

#include "krit/math/Point.h"
#include "krit/render/DrawCommand.h"
#include "krit/Camera.h"

namespace krit {

class App;
class Engine;

struct RenderContext {
    double elapsed;
    unsigned int frameId;
    App *app = nullptr;
    Engine *engine = nullptr;
    IntDimensions *window = nullptr;
    DrawCommandBuffer *drawCommandBuffer = nullptr;
    Camera *camera = nullptr;
    CameraTransform *transform = nullptr;
    Point offset;
    void *userData;
    bool debugDraw = false;

    RenderContext() {}

    template <typename T> T *data() { return static_cast<T*>(this->userData); }

    void setClip(Rectangle rect) {
        this->transformRect(rect);
        this->drawCommandBuffer->setClip(rect);
    }

    void clearClip() {
        Rectangle r;
        this->drawCommandBuffer->setClip(r);
    }

    void addRect(DrawKey &key, IntRectangle &rect, Matrix &matrix, Color color) {
        this->transformMatrix(matrix);
        this->drawCommandBuffer->addRect(key, rect, matrix, color);
    }

    void addRectRaw(DrawKey &key, IntRectangle &rect, Matrix &matrix, Color color) {
        this->drawCommandBuffer->addRect(key, rect, matrix, color);
    }

    void addTriangle(DrawKey &key, Triangle &t, Triangle &uv, Color color) {
        this->transformTriangle(t);
        this->drawCommandBuffer->addTriangle(key, t, uv, color);
    }

    void addTriangleRaw(DrawKey &key, Triangle &t, Triangle &uv, Color color) {
        this->drawCommandBuffer->addTriangle(key, t, uv, color);
    }

    Point &transformPoint(Point &point) {
        point.add(this->offset);
        if (this->camera) {
            this->camera->transformPoint(this->transform, point);
        }
        return point;
    }

    Point &untransformPoint(Point &point) {
        if (this->camera) {
            this->camera->untransformPoint(this->transform, point);
        }
        point.subtract(this->offset);
        return point;
    }

    Rectangle &transformRect(Rectangle &rect) {
        if (this->camera) {
            Point p(rect.x, rect.y);
            Dimensions d(rect.width, rect.height);
            this->transformPoint(p);
            this->transformDimensions(d);
            rect.setTo(p.x, p.y, d.width(), d.height());
        }
        return rect;
    }

    // TODO: untransformRect

    Triangle &transformTriangle(Triangle &t) {
        this->transformPoint(t.p1);
        this->transformPoint(t.p2);
        this->transformPoint(t.p3);
        return t;
    }

    // TODO: untransformTriangle

    Matrix &transformMatrix(Matrix &m) {
        m.translate(this->offset.x, this->offset.y);
        if (this->camera) {
            this->camera->transformMatrix(this->transform, m);
        }
        return m;
    }

    // TODO: untransformMatrix

    Dimensions &transformDimensions(Dimensions &d) {
        if (this->camera) {
            this->camera->scaleDimensions(this->transform, d);
        }
        return d;
    }

    Dimensions &untransformDimensions(Dimensions &d) {
        if (this->camera) {
            this->camera->unscaleDimensions(this->transform, d);
        }
        return d;
    }
};

}
#endif
