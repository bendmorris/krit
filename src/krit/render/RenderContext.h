#ifndef KRIT_RENDER_RENDER_CONTEXT
#define KRIT_RENDER_RENDER_CONTEXT

#include "krit/math/Point.h"
#include "krit/Camera.h"
#include "krit/render/DrawKey.h"
#include "krit/UpdateContext.h"

namespace krit {

struct App;
struct Engine;
struct DrawCommandBuffer;

struct RenderContext: public UpdateContext {
    DrawCommandBuffer *drawCommandBuffer = nullptr;
    CameraTransform *transform = nullptr;
    Point offset;
    bool debugDraw = false;

    RenderContext() {}

    void setClip(Rectangle rect);
    void clearClip();
    void addRect(DrawKey &key, IntRectangle &rect, Matrix &matrix, Color color);
    void addRectRaw(DrawKey &key, IntRectangle &rect, Matrix &matrix, Color color);
    void addTriangle(DrawKey &key, Triangle &t, Triangle &uv, Color color);
    void addTriangleRaw(DrawKey &key, Triangle &t, Triangle &uv, Color color);

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
