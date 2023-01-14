#ifndef KRIT_RENDER_DRAWCOMMAND
#define KRIT_RENDER_DRAWCOMMAND

#include "krit/math/Rectangle.h"
#include "krit/render/CommandBuffer.h"
#include "krit/render/DrawCall.h"
#include "krit/render/SceneShader.h"
#include "krit/utils/Color.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <utility>
#include <vector>

struct ImDrawData;

namespace krit {

struct RenderContext;
struct Matrix4;
struct FrameBuffer;
struct DrawKey;
struct Shader;
struct Triangle;

enum DrawCommandType {
    DrawTriangles,
    PushClipRect,
    PopClipRect,
    SetRenderTarget,
    DrawSceneShader,
    ClearColor,
    RenderImGui,

    DrawCommandTypeCount
};

struct SetRenderTargetArgs {
    FrameBuffer *target = nullptr;
    bool clear = false;

    SetRenderTargetArgs() {}
    SetRenderTargetArgs(FrameBuffer *target, bool clear)
        : target(target), clear(clear) {}
};

struct AutoClipBounds {
    std::pair<float, float> xRange{NAN, NAN};
    std::pair<float, float> yRange{NAN, NAN};
    std::pair<float, float> zRange{NAN, NAN};
    float xBuffer;
    float yBuffer;
    size_t clipIndex;
};

struct DrawCommandBuffer {
    std::vector<float> triangles;
    std::vector<AutoClipBounds> boundsStack;
    CommandBuffer<DrawCall, Rectangle, char, SetRenderTargetArgs, SceneShader *,
                  Color, ImDrawData *>
        buf;
    FrameBuffer *currentRenderTarget = nullptr;
    SpriteShader *defaultTextureShader = nullptr;
    SpriteShader *defaultColorShader = nullptr;

    DrawCommandBuffer() {
        triangles.reserve(0x20000);
        buf.get<DrawTriangles>().reserve(0x80);
        buf.get<PushClipRect>().reserve(0x10);
        buf.get<PopClipRect>().reserve(0x10);
        buf.get<SetRenderTarget>().reserve(0x10);
        buf.get<DrawSceneShader>().reserve(0x10);
        buf.get<ClearColor>().reserve(0x10);
    }

    virtual ~DrawCommandBuffer() {}

    DrawCall &getDrawCall(const DrawKey &key, int zIndex = 0);

    void clear() {
        triangles.clear();
        buf.clear();
        currentRenderTarget = nullptr;
    }

    void addTriangle(RenderContext &ctx, const DrawKey &key, const Triangle &t,
                     const Triangle &uv, const Color &color, int zIndex = 0);
    void addTriangle(RenderContext &ctx, const DrawKey &key, const Triangle &t,
                     const Triangle &uv, const Color &color1,
                     const Color &color2, const Color &color3, int zIndex = 0);

    void pushClip(Rectangle clip) { buf.emplace_back<PushClipRect>(clip); }
    void popClip() { buf.emplace_back<PopClipRect>(); }

    void startAutoClip(float xBuffer = 0, float yBuffer = 0);
    bool endAutoClip(RenderContext &ctx);

    void setRenderTarget(FrameBuffer *fb = nullptr, bool clear = false) {
        buf.emplace_back<SetRenderTarget>(currentRenderTarget = fb, clear);
    }

    void clearColor(Color color = Color::black()) {
        buf.emplace_back<ClearColor>(color);
    }

    void drawSceneShader(SceneShader *shader) {
        buf.emplace_back<DrawSceneShader>(shader);
    }

    void addTriangle(DrawCall &draw, const Triangle &t, const Triangle &uv,
                     const Color &color1, const Color &color2,
                     const Color &color3);

    void addTriangle(DrawCall &draw, float x1, float y1, float z1, float x2,
                     float y2, float z2, float x3, float y3, float z3,
                     float uv1, float uv2, float uv3, float uv4, float uv5,
                     float uv6, const Color &color);

    void addRect(RenderContext &ctx, const DrawKey &key,
                 const IntRectangle &rect, const Matrix4 &matrix,
                 const Color &color, int zIndex = 0);

    void updateBounds(AutoClipBounds &bounds, float x1, float y1, float x2,
                      float y2, float z1, float z2);
};

}

#endif
