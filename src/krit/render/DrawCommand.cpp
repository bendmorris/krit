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
                        auto &call = buf.emplace<DrawTriangles>(
                            typeIndex + 1, drawCallIndex + 1);
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
                // we can't fall through anymore, as there's a non-DrawTriangles
                // command here
                auto &call = buf.emplace<DrawTriangles>(typeIndex + 1,
                                                        drawCallIndex + 1);
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
    if (color.a > 0 || key.shader) {
        DrawCall &call = this->getDrawCall(key, zIndex);
        addTriangle(call, t, uv, color);
    }
}

}
