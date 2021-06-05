#ifndef KRIT_RENDER_DRAW_CALL
#define KRIT_RENDER_DRAW_CALL

#include "krit/Math.h"
#include "krit/render/BlendMode.h"
#include "krit/render/DrawKey.h"
#include "krit/render/ImageData.h"
#include "krit/render/Shader.h"
#include "krit/render/SmoothingMode.h"
#include "krit/utils/Color.h"
#include <memory>
#include <vector>

namespace krit {

struct TriangleData {
    Triangle t;
    Triangle uv;
    Color color;

    TriangleData() : t(Triangle()), uv(Triangle()), color(0) {}
    TriangleData(const Triangle &t, const Triangle &uv, const Color &color) : t(t), uv(uv), color(color) {}
    TriangleData(float t1, float t2, float t3, float t4, float t5, float t6, float uv1, float uv2, float uv3, float uv4, float uv5, float uv6, const Color &color):
        t(t1, t2, t3, t4, t5, t6), uv(uv1, uv2, uv3, uv4, uv5, uv6), color(color) {}

    /**
     * Returns the AABB containing this triangle.
     */
    Rectangle bounds();
};

struct DrawCall {
    DrawKey key;
    std::vector<TriangleData> data;

    DrawCall() {}

    DrawCall(DrawKey &key) : key(key) {
        this->data.reserve(0x1000);
    }

    size_t length() { return this->data.size(); }

    bool matches(const DrawKey &other) {
        return this->key == other;
    }

    void reset() {
        this->data.clear();
    }

    void addTriangle(const Triangle &t, const Triangle &uv, const Color &color) {
        this->data.emplace_back(t, uv, color);
    }

    inline void addTriangle(float t1, float t2, float t3, float t4, float t5, float t6, float uv1, float uv2, float uv3, float uv4, float uv5, float uv6, const Color &color) {
        this->data.emplace_back(t1, t2, t3, t4, t5, t6, uv1, uv2, uv3, uv4, uv5, uv6, color);
    }
};

}
#endif
