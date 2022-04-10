#include "krit/render/RenderContext.h"
#include "krit/render/DrawCommand.h"

namespace krit {

struct Matrix;
struct DrawKey;

IntDimensions RenderContext::size() {
    if (drawCommandBuffer->currentRenderTarget) {
        IntDimensions d(drawCommandBuffer->currentRenderTarget->size);
        d.x() *= drawCommandBuffer->currentRenderTarget->scale.x();
        d.y() *= drawCommandBuffer->currentRenderTarget->scale.y();
        return d;
    } else {
        return window->size();
    }
}

void RenderContext::pushClip(Rectangle &rect) {
    this->transformRect(rect);
    this->drawCommandBuffer->pushClip(rect);
}

void RenderContext::pushDynamicClip(Rectangle &rect) {
    this->drawCommandBuffer->pushDynamicClip(rect);
}

void RenderContext::popClip() { this->drawCommandBuffer->popClip(); }

void RenderContext::pushBounds(Rectangle &rect) {
    this->drawCommandBuffer->pushBounds(rect);
}

void RenderContext::popBounds() { this->drawCommandBuffer->popBounds(); }

void RenderContext::addRect(const DrawKey &key, IntRectangle &rect,
                            Matrix4 &matrix, Color color, int zIndex) {
    this->drawCommandBuffer->addRect(*this, key, rect, matrix, color, zIndex);
}

void RenderContext::addTriangle(const DrawKey &key, Triangle &t, Triangle &uv,
                                Color color, int zIndex) {
    this->drawCommandBuffer->addTriangle(*this, key, t, uv, color, zIndex);
}

void RenderContext::drawRect(int x, int y, int w, int h, Color c, float alpha) {
    c.a = alpha;
    IntRectangle r(0, 0, w, h);
    Matrix4 m;
    m.identity();
    m.translate(x, y);
    addRect(DrawKey(), r, m, c);
}

}
