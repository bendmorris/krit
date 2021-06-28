#include <algorithm>

#include "krit/render/DrawCall.h"
#include "krit/render/DrawCommand.h"

namespace krit {

Rectangle TriangleData::bounds() {
    return t.bounds();
}

void DrawCall::addTriangle(DrawCommandBuffer *buf, const Triangle &t, const Triangle &uv, const Color &color) {
    buf->triangles.emplace_back(t, uv, color);
    indices.push_back(buf->triangles.size() - 1);
}

void DrawCall::addTriangle(DrawCommandBuffer *buf, float t1, float t2, float t3, float t4, float t5, float t6, float uv1, float uv2, float uv3, float uv4, float uv5, float uv6, const Color &color) {
    buf->triangles.emplace_back(t1, t2, t3, t4, t5, t6, uv1, uv2, uv3, uv4, uv5, uv6, color);
    indices.push_back(buf->triangles.size() - 1);
}

}
