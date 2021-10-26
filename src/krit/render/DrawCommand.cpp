#include "krit/render/DrawCommand.h"

#include <memory>

#include "krit/math/Matrix.h"
#include "krit/render/DrawKey.h"
#include "krit/render/ImageData.h"

namespace krit {
struct Triangle;

DrawCall &DrawCommandBuffer::getDrawCall(const DrawKey &key, int zIndex) {
    auto &types = this->buf.commandTypes;
    auto &drawCalls = buf.get<DrawTriangles>();
    int drawCallIndex = drawCalls.size() - 1;
    if (!types.empty() && types.back() == DrawTriangles) {
        bool fallingThrough = false;
        for (int typeIndex = types.size() - 1; typeIndex > -1; --typeIndex) {
            if (types[typeIndex] == DrawTriangles) {
                auto &drawCall = drawCalls[drawCallIndex];
                if (drawCall.matches(key)) {
                    return drawCall;
                }
                if (drawCall.zIndex <= zIndex) {
                    if (fallingThrough) {
                        // not a higher z index; we can't fall through anymore
                        auto &call = buf.emplace<DrawTriangles>(typeIndex + 1, drawCallIndex + 1);
                        call.key = key;
                        call.zIndex = zIndex;
                        return call;
                    } else {
                        break;
                    }
                }
                --drawCallIndex;
                fallingThrough = true;
            } else {
                // we can't fall through anymore, as there's a non-DrawTriangles command here
                auto &call = buf.emplace<DrawTriangles>(typeIndex + 1, drawCallIndex + 1);
                call.key = key;
                call.zIndex = zIndex;
                return call;
            }
        }
    }
    auto &call = buf.emplace_back<DrawTriangles>();
    call.key = key;
    call.zIndex = zIndex;
    return call;
}

void DrawCommandBuffer::addTriangle(RenderContext &ctx, const DrawKey &key,
                                    const Triangle &t, const Triangle &uv,
                                    const Color &color, int zIndex) {
    if (color.a > 0) {
        DrawCall &call = this->getDrawCall(key, zIndex);
        addTriangle(call, t, uv, color);
    }
}

void DrawCommandBuffer::addRect(RenderContext &ctx, const DrawKey &key,
                                const IntRectangle &rect, const Matrix &matrix,
                                const Color &color, int zIndex) {
    if (color.a <= 0) {
        return;
    }

    DrawCall &call = this->getDrawCall(key, zIndex);

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

    addTriangle(call, matrix.tx, matrix.ty, xa, yb, xc, yd, uvx1, uvy1, uvx2,
                uvy1, uvx1, uvy2, color);
    addTriangle(call, xc, yd, xa, yb, xa + rect.height * matrix.c,
                yb + rect.height * matrix.d, uvx1, uvy2, uvx2, uvy1, uvx2, uvy2,
                color);
}

}
