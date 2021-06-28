#include "krit/render/RenderContext.h"

#include "krit/render/DrawCommand.h"

namespace krit {

struct Matrix;
struct DrawKey;

void RenderContext::pushClip(Rectangle rect) {
    this->transformRect(rect);
    this->drawCommandBuffer->pushClip(rect);
}

void RenderContext::popClip() {
    this->drawCommandBuffer->popClip();
}

void RenderContext::addRect(DrawKey &key, IntRectangle &rect, Matrix &matrix, Color color) {
    this->transformMatrix(matrix);
    this->drawCommandBuffer->addRect(*this, key, rect, matrix, color);
}

void RenderContext::addRectRaw(DrawKey &key, IntRectangle &rect, Matrix &matrix, Color color) {
    this->drawCommandBuffer->addRect(*this, key, rect, matrix, color);
}

void RenderContext::addTriangle(DrawKey &key, Triangle &t, Triangle &uv, Color color) {
    this->transformTriangle(t);
    this->drawCommandBuffer->addTriangle(*this, key, t, uv, color);
}

void RenderContext::addTriangleRaw(DrawKey &key, Triangle &t, Triangle &uv, Color color) {
    this->drawCommandBuffer->addTriangle(*this, key, t, uv, color);
}

}