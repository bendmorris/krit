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
#include <cassert>

namespace krit {

struct DrawCommandBuffer;
struct Matrix;
struct DrawKey;

struct RenderContext {
    DrawCommandBuffer *drawCommandBuffer = nullptr;
    Camera *camera = nullptr;
    bool debugDraw = false;

    RenderContext() {}

    IntDimensions size();

    int width() { return size().x; }
    int height() { return size().y; }

    void pushClip(Rectangle &rect);
    void popClip();
    void startAutoClip(float xBuffer = 0, float yBuffer = 0);
    bool endAutoClip();

    void drawRect(int x, int y, int w, int h, Color c = Color::white(),
                  float alpha = 1);
};

}
#endif
