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
    FrameBuffer *fb{nullptr};
    Vec2i pos;

    ReadPixelArgs() {}
    ReadPixelArgs(FrameBuffer *fb, int x, int y) : fb(fb), pos(x, y) {}
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
    std::array<float, 4> position;
    std::array<float, 2> texCoord;
    uint32_t color;

    VertexData() {}
    VertexData(float x, float y, float z, float w, float uvx, float uvy,
               float r, float g, float b, float a)
        : position{x, y, z, w}, texCoord{uvx, uvy},
          color(Color(r, g, b, a).argb()) {}

    inline void setPosition(float x, float y, float z, float w) {
        position[0] = x;
        position[1] = y;
        position[2] = z;
        position[3] = w;
    }
    inline void setTexCoord(float uvx, float uvy) {
        texCoord[0] = uvx;
        texCoord[1] = uvy;
    }
    inline void setColor(const Color &c) { color = c.abgr(); }
    inline void setColor(uint32_t c) { color = c; }
};

struct DrawCommandBuffer {
    static inline void addVertex(VertexData &to, float x, float y, float z,
                                 float uvx, float uvy, const Color &c) {
        to.setPosition(x, y, z, 1.0);
        to.setTexCoord(uvx, uvy);
        to.setColor(c);
    }

    inline void addVertex(VertexData &to, const Vec3f &t, const Vec3f &uv,
                          const Color &c) {
        to.setPosition(t.x, t.y, t.z, 1.0);
        to.setTexCoord(uv.x, uv.y);
        to.setColor(c);
    }

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

    inline void addTriangle(const DrawKey &key, const Triangle &t,
                            const Triangle &uv, const Color &color,
                            int zIndex = 0) {
        addTriangle(getDrawCall(key, zIndex), t, uv, color, color, color);
    }
    inline void addTriangle(DrawCall &draw, const Triangle &t,
                            const Triangle &uv, const Color &color) {
        addTriangle(draw, t, uv, color, color, color);
    }

    inline void addTriangle(const DrawKey &key, const Triangle &t,
                            const Triangle &uv, const Color &color1,
                            const Color &color2, const Color &color3,
                            int zIndex = 0) {
        addTriangle(getDrawCall(key, zIndex), t, uv, color1, color2, color3);
    }
    inline void addTriangle(DrawCall &draw, const Triangle &t,
                            const Triangle &uv, const Color &color1,
                            const Color &color2, const Color &color3) {
        size_t i = vertexData.size();
        vertexData.resize(i + 3);
        addVertex(vertexData[i], t.p1, uv.p1, color1);
        addVertex(vertexData[i + 1], t.p2, uv.p2, color2);
        addVertex(vertexData[i + 2], t.p3, uv.p3, color3);
        draw.indices.push_back(i);
        draw.indices.push_back(i + 1);
        draw.indices.push_back(i + 2);

        if (boundsStack.size()) {
            float x1 = std::min({t.p1.x, t.p2.x, t.p3.x});
            float x2 = std::max({t.p1.x, t.p2.x, t.p3.x});
            float y1 = std::min({t.p1.y, t.p2.y, t.p3.y});
            float y2 = std::max({t.p1.y, t.p2.y, t.p3.y});
            float z1 = std::min({t.p1.z, t.p2.z, t.p3.z});
            float z2 = std::max({t.p1.z, t.p2.z, t.p3.z});
            for (auto &b : boundsStack) {
                updateBounds(b, x1, y1, x2, y2, z1, z2);
            }
        }
    }

    inline VertexData *emplaceTriangle(const DrawKey &key, int zIndex = 0) {
        return emplaceTriangle(getDrawCall(key, zIndex));
    }
    inline VertexData *emplaceTriangle(DrawCall &draw) {
        size_t i = vertexData.size();
        vertexData.resize(i + 3);
        draw.indices.push_back(i);
        draw.indices.push_back(i + 1);
        draw.indices.push_back(i + 2);
        return &vertexData[i];
    }

    inline void addRect(const DrawKey &key, const IntRectangle &rect,
                        const Matrix4 &matrix, const Color &color,
                        int zIndex = 0) {
        addRect(getDrawCall(key, zIndex), rect, matrix, color);
    }
    inline void addRect(DrawCall &draw, const IntRectangle &rect,
                        const Matrix4 &matrix, const Color &color) {
        float uvx1;
        float uvy1;
        float uvx2;
        float uvy2;
        std::shared_ptr<ImageData> imageData = draw.key.image;
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
        Vec4f ul(0, 0, 0, 1), ur(rect.width, 0, 0, 1), ll(0, rect.height, 0, 1),
            lr(rect.width, rect.height, 0, 1);
        ul = matrix * ul;
        ur = matrix * ur;
        ll = matrix * ll;
        lr = matrix * lr;

        size_t i = vertexData.size();
        vertexData.resize(i + 4);
        addVertex(vertexData[i], ul.x, ul.y, ul.z, uvx1, uvy1, color);
        addVertex(vertexData[i + 1], ur.x, ur.y, ur.z, uvx2, uvy1, color);
        addVertex(vertexData[i + 2], ll.x, ll.y, ll.z, uvx1, uvy2, color);
        addVertex(vertexData[i + 3], lr.x, lr.y, lr.z, uvx2, uvy2, color);

        draw.indices.push_back(i);
        draw.indices.push_back(i + 1);
        draw.indices.push_back(i + 2);
        draw.indices.push_back(i + 2);
        draw.indices.push_back(i + 1);
        draw.indices.push_back(i + 3);

        if (boundsStack.size()) {
            float x1 = std::min({ll.x, lr.x, ul.x, ur.x});
            float x2 = std::max({ll.x, lr.x, ul.x, ur.x});
            float y1 = std::min({ll.y, lr.y, ul.y, ur.y});
            float y2 = std::max({ll.y, lr.y, ul.y, ur.y});
            float z1 = std::min({ll.z, lr.z, ul.z, ur.z});
            float z2 = std::max({ll.z, lr.z, ul.z, ur.z});
            for (auto &b : boundsStack) {
                updateBounds(b, x1, y1, x2, y2, z1, z2);
            }
        }
    }

    void updateBounds(AutoClipBounds &bounds, float x1, float y1, float x2,
                      float y2, float z1, float z2);

    void setCamera(Camera *c) { buf.emplace_back<SetCamera>(c); }

    void queueReadPixel(FrameBuffer *fb, int x, int y) {
        buf.emplace_back<ReadPixel>(fb, x, y);
    }
};

}

#endif
