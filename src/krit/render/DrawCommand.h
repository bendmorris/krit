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
    SetCamera,
    DrawTriangles,
    PushClipRect,
    PopClipRect,
    SetRenderTarget,
    DrawSceneShader,
    ClearColor,
    ReadPixel,
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

struct ReadPixelArgs {
    FrameBuffer *fb { nullptr };
    Vec2i pos;

    ReadPixelArgs() {}
    ReadPixelArgs(FrameBuffer *fb, int x, int y): fb(fb), pos(x, y) {}
};

struct AutoClipBounds {
    std::pair<float, float> xRange{NAN, NAN};
    std::pair<float, float> yRange{NAN, NAN};
    std::pair<float, float> zRange{NAN, NAN};
    float xBuffer;
    float yBuffer;
    size_t clipIndex;
};

struct VertexData {
    Vec4f position;
    Vec2f texCoord;
    Color color;

    VertexData() {}
    VertexData(float x, float y, float z, float w, float uvx, float uvy,
               float r, float g, float b, float a)
        : position(x, y, z, w), texCoord(uvx, uvy), color(r, g, b, a) {}
};

struct DrawCommandBuffer {
    std::vector<VertexData> vertexData;
    std::vector<AutoClipBounds> boundsStack;
    CommandBuffer<Camera *, DrawCall, Rectangle, char, SetRenderTargetArgs,
                  SceneShader *, Color, ReadPixelArgs, ImDrawData *>
        buf;
    FrameBuffer *currentRenderTarget = nullptr;
    SpriteShader *defaultTextureShader = nullptr;
    SpriteShader *defaultColorShader = nullptr;

    DrawCommandBuffer();

    virtual ~DrawCommandBuffer() {}

    DrawCall &getDrawCall(const DrawKey &key, int zIndex = 0);

    DrawCommandBuffer &operator+=(DrawCommandBuffer &other);

    void clear();

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

    void setCamera(Camera *c) { buf.emplace_back<SetCamera>(c); }

    void queueReadPixel(FrameBuffer *fb, int x, int y) { buf.emplace_back<ReadPixel>(fb, x, y); }
};

}

#endif
