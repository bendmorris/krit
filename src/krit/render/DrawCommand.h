#ifndef KRIT_RENDER_DRAWCOMMAND
#define KRIT_RENDER_DRAWCOMMAND

#include "krit/math/Rectangle.h"
#include "krit/render/CommandBuffer.h"
#include "krit/render/DrawCall.h"
#include "krit/render/SceneShader.h"
#include "krit/utils/Color.h"
#include <algorithm>
#include <vector>

struct ImDrawData;

namespace krit {

struct RenderContext;
struct Matrix;
struct FrameBuffer;
struct DrawKey;
struct Shader;
struct Triangle;

enum DrawCommandType {
    DrawTriangles,
    PushClipRect,
    PushDynamicClipRect,
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

struct DrawCommandBuffer {
    std::vector<float> triangles;
    std::vector<Rectangle *> boundsStack;
    CommandBuffer<DrawCall, Rectangle, Rectangle *, char, SetRenderTargetArgs,
                  SceneShader *, Color, ImDrawData *>
        buf;
    FrameBuffer *currentRenderTarget = nullptr;
    SpriteShader *defaultTextureShader = nullptr;
    SpriteShader *defaultColorShader = nullptr;

    DrawCommandBuffer() {
        triangles.reserve(0x20000);
        buf.get<DrawTriangles>().reserve(0x80);
        buf.get<PushClipRect>().reserve(0x10);
        buf.get<PushDynamicClipRect>().reserve(0x10);
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

    void pushClip(Rectangle &clip) { buf.emplace_back<PushClipRect>(clip); }

    void pushDynamicClip(Rectangle &clip) {
        buf.emplace_back<PushDynamicClipRect>(&clip);
    }

    void popClip() { buf.emplace_back<PopClipRect>(); }

    void pushBounds(Rectangle &r) { boundsStack.emplace_back(&r); }

    void popBounds() { boundsStack.pop_back(); }

    void setRenderTarget(FrameBuffer *fb = nullptr, bool clear = false) {
        buf.emplace_back<SetRenderTarget>(currentRenderTarget = fb, clear);
    }

    void clearColor() { buf.emplace_back<ClearColor>(Color(0, 0, 0, 0)); }
    void clearColor(const Color &color) { buf.emplace_back<ClearColor>(color); }
    void clearColor(float r, float g, float b, float a) {
        buf.emplace_back<ClearColor>(Color(r, g, b, a));
    }

    void drawSceneShader(SceneShader *shader) {
        buf.emplace_back<DrawSceneShader>(shader);
    }

    void addTriangle(DrawCall &draw, const Triangle &t, const Triangle &uv,
                     const Color &color) {
        size_t i = triangles.size();
        triangles.resize(i + 24);
        SpriteShader *s =
            draw.key.shader
                ? draw.key.shader
                : draw.key.image ? defaultTextureShader : defaultColorShader;
        s->prepareVertex(triangles.data() + i, t.p1.x, t.p1.y, uv.p1.x, uv.p1.y,
                         color);
        s->prepareVertex(triangles.data() + i + 8, t.p2.x, t.p2.y, uv.p2.x,
                         uv.p2.y, color);
        s->prepareVertex(triangles.data() + i + 16, t.p3.x, t.p3.y, uv.p3.x,
                         uv.p3.y, color);
        draw.indices.push_back(i / 8);
        draw.indices.push_back(i / 8 + 1);
        draw.indices.push_back(i / 8 + 2);

        if (boundsStack.size()) {
            float x1 = std::min({ t.p1.x, t.p2.x, t.p3.x });
            float x2 = std::max({ t.p1.x, t.p2.x, t.p3.x });
            float y1 = std::min({ t.p1.y, t.p2.y, t.p3.y });
            float y2 = std::max({ t.p1.y, t.p2.y, t.p3.y });
            for (auto b : boundsStack) {
                updateBounds(*b, x1, y1, x2, y2);
            }
        }
    }

    void addTriangle(DrawCall &draw, float t1, float t2, float t3, float t4,
                     float t5, float t6, float uv1, float uv2, float uv3,
                     float uv4, float uv5, float uv6, const Color &color) {
        size_t i = triangles.size();
        triangles.resize(i + 24);
        SpriteShader *s =
            draw.key.shader
                ? draw.key.shader
                : draw.key.image ? defaultTextureShader : defaultColorShader;
        s->prepareVertex(triangles.data() + i, t1, t2, uv1, uv2, color);
        s->prepareVertex(triangles.data() + i + 8, t3, t4, uv3, uv4, color);
        s->prepareVertex(triangles.data() + i + 16, t5, t6, uv5, uv6, color);
        draw.indices.push_back(i / 8);
        draw.indices.push_back(i / 8 + 1);
        draw.indices.push_back(i / 8 + 2);

        if (boundsStack.size()) {
            float x1 = std::min({ t1, t3, t5 });
            float x2 = std::max({ t1, t3, t5 });
            float y1 = std::min({ t2, t4, t6 });
            float y2 = std::max({ t2, t4, t6 });
            for (auto b : boundsStack) {
                updateBounds(*b, x1, y1, x2, y2);
            }
        }
    }

    void addRect(RenderContext &ctx, const DrawKey &key,
                 const IntRectangle &rect, const Matrix &matrix,
                 const Color &color, int zIndex = 0) {
        if (color.a <= 0 && !key.shader) {
            return;
        }

        DrawCall &draw = this->getDrawCall(key, zIndex);

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
            uvx2 =
                (rect.x + rect.width) / static_cast<float>(imageData->width());
            uvy2 = (rect.y + rect.height) /
                   static_cast<float>(imageData->height());
        }

        // matrix transformations
        float xa = rect.width * matrix.a + matrix.tx;
        float yb = rect.width * matrix.b + matrix.ty;
        float xc = rect.height * matrix.c + matrix.tx;
        float yd = rect.height * matrix.d + matrix.ty;

        size_t i = triangles.size();
        triangles.resize(i + 32);
        SpriteShader *s =
            draw.key.shader
                ? draw.key.shader
                : draw.key.image ? defaultTextureShader : defaultColorShader;
        s->prepareVertex(triangles.data() + i, matrix.tx, matrix.ty, uvx1, uvy1,
                         color);
        s->prepareVertex(triangles.data() + i + 8, xa, yb, uvx2, uvy1, color);
        s->prepareVertex(triangles.data() + i + 16, xc, yd, uvx1, uvy2, color);
        s->prepareVertex(triangles.data() + i + 24, xa + rect.height * matrix.c,
                         yb + rect.height * matrix.d, uvx2, uvy2, color);
        draw.indices.push_back(i / 8);
        draw.indices.push_back(i / 8 + 1);
        draw.indices.push_back(i / 8 + 2);
        draw.indices.push_back(i / 8 + 2);
        draw.indices.push_back(i / 8 + 1);
        draw.indices.push_back(i / 8 + 3);

        if (boundsStack.size()) {
            float x1 = std::min({ matrix.tx, xa, xc, matrix.tx, xa + rect.height * matrix.c });
            float x2 = std::max({ matrix.tx, xa, xc, matrix.tx, xa + rect.height * matrix.c });
            float y1 = std::min({ matrix.ty, yb, yd, yb + rect.height * matrix.d });
            float y2 = std::max({ matrix.ty, yb, yd, yb + rect.height * matrix.d });
            for (auto b : boundsStack) {
                updateBounds(*b, x1, y1, x2, y2);
            }
        }
    }

    void updateBounds(Rectangle &bounds, float x1, float y1, float x2,
                      float y2) {
        if (!!bounds) {
            x1 = std::min(bounds.x, x1);
            x2 = std::max(bounds.right(), x2);
            y1 = std::min(bounds.y, y1);
            y2 = std::max(bounds.bottom(), y2);
        }
        bounds.setTo(x1, y1, x2 - x1, y2 - y1);
    }
};

}

#endif
