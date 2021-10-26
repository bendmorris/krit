#include "krit/render/RenderContext.h"
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
    } else {
        return window->size();
    }
}

void RenderContext::pushClip(Rectangle rect) {
    this->transformRect(rect);
    this->drawCommandBuffer->pushClip(rect);
}

void RenderContext::popClip() { this->drawCommandBuffer->popClip(); }

void RenderContext::addRect(DrawKey &key, IntRectangle &rect, Matrix &matrix,
                            Color color, int zIndex) {
    this->transformMatrix(matrix);
    this->drawCommandBuffer->addRect(*this, key, rect, matrix, color, zIndex);
}

void RenderContext::addRectRaw(DrawKey &key, IntRectangle &rect, Matrix &matrix,
                               Color color, int zIndex) {
    this->drawCommandBuffer->addRect(*this, key, rect, matrix, color, zIndex);
}

void RenderContext::addTriangle(DrawKey &key, Triangle &t, Triangle &uv,
                                Color color, int zIndex) {
    this->transformTriangle(t);
    this->drawCommandBuffer->addTriangle(*this, key, t, uv, color, zIndex);
}

void RenderContext::addTriangleRaw(DrawKey &key, Triangle &t, Triangle &uv,
                                   Color color, int zIndex) {
    this->drawCommandBuffer->addTriangle(*this, key, t, uv, color, zIndex);
}

}