#ifndef KRIT_RENDER_DRAW_CALL
#define KRIT_RENDER_DRAW_CALL

#include "krit/math/Rectangle.h"
#include "krit/math/Triangle.h"
#include "krit/render/DrawKey.h"
#include "krit/utils/Color.h"
#include <memory>
#include <stddef.h>
#include <vector>

namespace krit {

struct DrawCommandBuffer;

struct TriangleData {
    Triangle t;
    Triangle uv;
    Color color;

    TriangleData() : t(Triangle()), uv(Triangle()), color(0) {}
    TriangleData(const Triangle &t, const Triangle &uv, const Color &color)
        : t(t), uv(uv), color(color) {}
    TriangleData(float t1, float t2, float t3, float t4, float t5, float t6,
                 float uv1, float uv2, float uv3, float uv4, float uv5,
                 float uv6, const Color &color)
        : t(t1, t2, t3, t4, t5, t6), uv(uv1, uv2, uv3, uv4, uv5, uv6),
          color(color) {}

    /**
     * Returns the AABB containing this triangle.
     */
    Rectangle bounds();
};

struct DrawCall {
    DrawKey key;
    std::vector<unsigned int> indices;

    DrawCall() {}

    DrawCall(DrawKey &key) : key(key) { this->indices.reserve(0x1000); }

    size_t length() { return this->indices.size(); }

    bool matches(const DrawKey &other) { return this->key == other; }

    void reset() { this->indices.clear(); }

    void addTriangle(DrawCommandBuffer *buf, const Triangle &t,
                     const Triangle &uv, const Color &color);
    void addTriangle(DrawCommandBuffer *buf, float t1, float t2, float t3,
                     float t4, float t5, float t6, float uv1, float uv2,
                     float uv3, float uv4, float uv5, float uv6,
                     const Color &color);
};

}
#endif
