#include "krit/render/DrawCommand.h"

#include <memory>

#include "krit/math/Matrix.h"
#include "krit/render/DrawKey.h"
#include "krit/render/ImageData.h"

namespace krit {
struct Triangle;

DrawCall &DrawCommandBuffer::getDrawCall(const DrawKey &key) {
    // TODO: fallthrough logic
    if (!this->buf.commandTypes.empty() &&
        this->buf.commandTypes.back() == DrawTriangles) {
        DrawCall &last = buf.get<DrawTriangles>().back();
        if (last.matches(key)) {
            return last;
        }
    }
    auto &call = buf.emplace_back<DrawTriangles>();
    call.key = key;
    return call;
}

void DrawCommandBuffer::addTriangle(RenderContext &ctx, const DrawKey &key,
                                    const Triangle &t, const Triangle &uv,
                                    const Color &color) {
    if (color.a > 0) {
        DrawCall &call = this->getDrawCall(key);
        call.addTriangle(this, t, uv, color);
    }
}

void DrawCommandBuffer::addRect(RenderContext &ctx, const DrawKey &key,
                                const IntRectangle &rect, const Matrix &matrix,
                                const Color &color) {
    if (color.a <= 0) {
        return;
    }

    DrawCall &call = this->getDrawCall(key);

    float uvx1;
    float uvy1;
    float uvx2;
    float uvy2;
    std::shared_ptr<ImageData> imageData = key.image;
    if (!imageData) {
        uvx1 = uvy1 = 0;
        uvx2 = rect.width;
        uvy2 = rect.height;
    } else {
        uvx1 = rect.x / static_cast<float>(imageData->width());
        uvy1 = rect.y / static_cast<float>(imageData->height());
        uvx2 = (rect.x + rect.width) / static_cast<float>(imageData->width());
        uvy2 = (rect.y + rect.height) / static_cast<float>(imageData->height());
    }

    // matrix transformations
    float xa = rect.width * matrix.a + matrix.tx;
    float yb = rect.width * matrix.b + matrix.ty;
    float xc = rect.height * matrix.c + matrix.tx;
    float yd = rect.height * matrix.d + matrix.ty;

    call.addTriangle(this, matrix.tx, matrix.ty, xa, yb, xc, yd, uvx1, uvy1,
                     uvx2, uvy1, uvx1, uvy2, color);
    call.addTriangle(this, xc, yd, xa, yb, xa + rect.height * matrix.c,
                     yb + rect.height * matrix.d, uvx1, uvy2, uvx2, uvy1, uvx2,
                     uvy2, color);
}

}
