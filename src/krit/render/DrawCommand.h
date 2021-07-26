#ifndef KRIT_RENDER_DRAWCOMMAND
#define KRIT_RENDER_DRAWCOMMAND

#include "krit/math/Rectangle.h"
#include "krit/render/CommandBuffer.h"
#include "krit/render/DrawCall.h"
#include "krit/render/Material.h"
#include "krit/utils/Color.h"
#include <algorithm>
#include <vector>

struct ImDrawData;

namespace krit {

struct RenderContext;
struct Matrix;
struct BaseFrameBuffer;
struct DrawKey;
struct Shader;
struct Triangle;

enum DrawCommandType {
    DrawTriangles,
    PushClipRect,
    PopClipRect,
    SetRenderTarget,
    DrawMaterial,
    ClearColor,
    RenderImGui,

    DrawCommandTypeCount
};

struct DrawCommandBuffer {
    std::vector<TriangleData> triangles;
    CommandBuffer<DrawCall, Rectangle, char, BaseFrameBuffer *, Material, Color,
                  ImDrawData *>
        buf;

    DrawCommandBuffer() {
        triangles.reserve(0x10000);
        buf.get<DrawTriangles>().reserve(0x40);
        buf.get<PushClipRect>().reserve(0x10);
        buf.get<PopClipRect>().reserve(0x10);
        buf.get<SetRenderTarget>().reserve(0x10);
        buf.get<DrawMaterial>().reserve(0x10);
        buf.get<ClearColor>().reserve(0x10);
    }

    DrawCall &getDrawCall(const DrawKey &key);

    void clear() {
        triangles.clear();
        buf.clear();
    }

    void addTriangle(RenderContext &ctx, const DrawKey &key, const Triangle &t,
                     const Triangle &uv, const Color &color);
    void addRect(RenderContext &ctx, const DrawKey &key,
                 const IntRectangle &rect, const Matrix &matrix,
                 const Color &color);

    void pushClip(Rectangle &clip) {
        auto &rect = buf.emplace_back<PushClipRect>();
        rect.setTo(clip);
    }

    void popClip() { buf.emplace_back<PopClipRect>(); }

    void setRenderTarget(BaseFrameBuffer *fb = nullptr) {
        buf.emplace_back<SetRenderTarget>(fb);
    }

    void clearColor(const Color &color) { buf.emplace_back<ClearColor>(color); }

    Material &addMaterial(Shader *shader) {
        return buf.emplace_back<DrawMaterial>(shader);
    }

    void addTriangle(DrawCall &draw, const Triangle &t, const Triangle &uv,
                     const Color &color) {
        triangles.emplace_back(t, uv, color);
        draw.indices.push_back(triangles.size() - 1);
    }
    void addTriangle(DrawCall &draw, float t1, float t2, float t3, float t4,
                     float t5, float t6, float uv1, float uv2, float uv3,
                     float uv4, float uv5, float uv6, const Color &color) {
        triangles.emplace_back(t1, t2, t3, t4, t5, t6, uv1, uv2, uv3, uv4, uv5,
                               uv6, color);
        draw.indices.push_back(triangles.size() - 1);
    }
};

}

#endif
