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

using namespace std;
using namespace krit;

namespace krit {

struct TriangleData {
    Triangle t;
    Triangle uv;
    Color color;

    TriangleData() : t(Triangle()), uv(Triangle()), color(0) {}
    TriangleData(Triangle &t, Triangle &uv, Color color) : t(t), uv(uv), color(color) {}

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
        this->data.reserve(0x100);
    }

    std::size_t length() { return this->data.size(); }

    bool matches(const DrawKey &other) {
        return this->key == other;
    }

    void reset() {
        this->data.clear();
    }

    void addTriangle(Triangle &t, Triangle &uv, Color color) {
        this->data.emplace_back(t, uv, color);
    }
};

}
#endif
