#ifndef KRIT_RENDER_RENDER_CONTEXT
#define KRIT_RENDER_RENDER_CONTEXT

#include "krit/Camera.h"
#include "krit/UpdateContext.h"
#include "krit/Window.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Point.h"
#include "krit/math/Rectangle.h"
#include "krit/math/Triangle.h"
#include "krit/utils/Color.h"

namespace krit {

struct DrawCommandBuffer;
struct Matrix;
struct DrawKey;

struct RenderContext : public UpdateContext {
    DrawCommandBuffer *drawCommandBuffer = nullptr;
    bool debugDraw = false;

    RenderContext() {}

    IntDimensions size();

    int width() { return size().x; }
    int height() { return size().y; }

    void pushClip(Rectangle &rect);
    void pushDynamicClip(Rectangle &rect);
    void popClip();

    void pushBounds(Rectangle &rect);
    void popBounds();

    void addRect(const DrawKey &key, IntRectangle &rect, Matrix &matrix, Color color, int zIndex = 0);
    void addRectRaw(const DrawKey &key, IntRectangle &rect, Matrix &matrix,
                    Color color, int zIndex = 0);
    void addTriangle(const DrawKey &key, Triangle &t, Triangle &uv, Color color, int zIndex = 0);
    void addTriangleRaw(const DrawKey &key, Triangle &t, Triangle &uv, Color color, int zIndex = 0);

    Point &transformPoint(Point &point) {
        if (this->camera) {
            this->camera->transformPoint(point);
        }
        return point;
    }

    Point &untransformPoint(Point &point) {
        if (this->camera) {
            this->camera->untransformPoint(point);
        }
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
        if (this->camera) {
            this->camera->transformMatrix(m);
        }
        return m;
    }

    // TODO: untransformMatrix

    Dimensions &transformDimensions(Dimensions &d) {
        if (this->camera) {
            this->camera->scaleDimensions(d);
        }
        return d;
    }

    Dimensions &untransformDimensions(Dimensions &d) {
        if (this->camera) {
            this->camera->unscaleDimensions(d);
        }
        return d;
    }

    void drawRect(int x, int y, int w, int h, Color c = Color::white(), float alpha = 1);
};

}
#endif
