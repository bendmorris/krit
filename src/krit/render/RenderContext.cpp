#include "krit/render/RenderContext.h"
#include "krit/Engine.h"
#include "krit/render/DrawCommand.h"

namespace krit {

struct Matrix;
struct DrawKey;

IntDimensions RenderContext::size() {
    if (drawCommandBuffer->currentRenderTarget) {
        IntDimensions d(drawCommandBuffer->currentRenderTarget->size);
        d.x *= drawCommandBuffer->currentRenderTarget->scale.x;
        d.y *= drawCommandBuffer->currentRenderTarget->scale.y;
        return d;
    } else if (camera) {
        return IntDimensions(camera->viewportWidth(), camera->viewportHeight());
    } else {
        return engine->window.size();
    }
}

void RenderContext::pushClip(Rectangle &rect) {
    assert(camera);
    this->camera->transformRect(rect);
    this->drawCommandBuffer->pushClip(rect);
}

void RenderContext::startAutoClip(float xBuffer, float yBuffer) {
    this->drawCommandBuffer->startAutoClip(xBuffer, yBuffer);
}

bool RenderContext::endAutoClip() {
    return this->drawCommandBuffer->endAutoClip(*this);
}

void RenderContext::popClip() { this->drawCommandBuffer->popClip(); }

void RenderContext::drawRect(int x, int y, int w, int h, Color c, float alpha) {
    c.a = alpha;
    IntRectangle r(0, 0, w, h);
    Matrix4 m;
    m.identity();
    m.translate(x, y);
    drawCommandBuffer->addRect(DrawKey(), r, m, c);
}

}
