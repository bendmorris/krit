#ifndef KRIT_RENDER_DRAWCONTEXT
#define KRIT_RENDER_DRAWCONTEXT

#include "krit/Math.h"
#include "krit/render/BlendMode.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Color.h"

namespace krit {

struct DrawContext {
    static DrawContext *create() {
        return new DrawContext();
    }

    BlendMode blend = BlendMode::Alpha;
    SmoothingMode smooth = SmoothingMode::SmoothLinear;
    SpriteShader *shader = nullptr;
    Color color;
    float alpha = 1;
    float lineThickness = 1;
    float pitch = 0;
    int zIndex = 0;

    DrawContext() {}

    void line(const Vec2f &p1, const Vec2f &p2);

    void polyline(const Vec2f *points, size_t pointsLength, bool miterJoint);

    void rect(const Rectangle &r);

    void rectFilled(const Rectangle &r);

    void circle(const Vec2f &center, float radius, size_t segments);
    void ring(const Vec2f &center, float radius, size_t segments);
    void circleFilled(const Vec2f &center, float radius, size_t segments);

    void arc(const Vec2f &center, float radius, float startRads,
             float angleRads, size_t segments = 16, float innerAlpha = 1);

    void drawTriangle(const Vec2f &v1, const Vec2f &v2, const Vec2f &v3);

    void drawQuad(float x1, float y1, float x2, float y2, float x3, float y3,
                  float x4, float y4);

    void addTriangle(float tx1, float ty1, float tx2, float ty2, float tx3,
                     float ty3, const Color &c1, const Color &c2,
                     const Color &c3);
};
}

#endif
