#ifndef KRIT_RENDER_DRAWCOMMAND
#define KRIT_RENDER_DRAWCOMMAND

#include "krit/Math.h"
#include "krit/render/BlendMode.h"
#include "krit/render/CommandBuffer.h"
#include "krit/render/DrawCall.h"
#include "krit/render/DrawKey.h"
#include "krit/render/FrameBuffer.h"
#include "krit/render/ImageData.h"
#include "krit/render/Material.h"
#include "krit/render/Shader.h"
#include "krit/render/SmoothingMode.h"
#include "krit/utils/Color.h"
#include <memory>
#include <tuple>
#include <vector>

struct SDL_Window;
struct ImDrawData;

namespace krit {

struct RenderContext;

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

struct DrawCommandBuffer: public CommandBuffer<
    DrawCall,
    Rectangle,
    char,
    BaseFrameBuffer*,
    Material,
    Color,
    ImDrawData*
> {
    DrawCommandBuffer() {
        this->get<DrawTriangles>().reserve(0x40);
        this->get<PushClipRect>().reserve(0x10);
        this->get<PopClipRect>().reserve(0x10);
        this->get<SetRenderTarget>().reserve(0x10);
        this->get<DrawMaterial>().reserve(0x10);
        this->get<ClearColor>().reserve(0x10);
    }

    DrawCall &getDrawCall(const DrawKey &key);

    void addTriangle(RenderContext &ctx, const DrawKey &key, const Triangle &t, const Triangle &uv, const Color &color);
    void addRect(RenderContext &ctx, const DrawKey &key, const IntRectangle &rect, const Matrix &matrix, const Color &color);

    void pushClip(Rectangle &clip) {
        auto &rect = this->emplace_back<PushClipRect>();
        rect.setTo(clip);
    }

    void popClip() {
        this->emplace_back<PopClipRect>();
    }

    void setRenderTarget(BaseFrameBuffer *fb = nullptr) {
        this->emplace_back<SetRenderTarget>(fb);
    }

    void clearColor(const Color &color) {
        this->emplace_back<ClearColor>(color);
    }

    Material &addMaterial(Shader *shader) {
        return this->emplace_back<DrawMaterial>(shader);
    }
};

}

#endif
