#include "krit/render/Renderer.h"
#if KRIT_ENABLE_TOOLS
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#endif
#include "krit/Engine.h"
#include "krit/editor/Editor.h"
#include "krit/render/BlendMode.h"
#include "krit/render/CommandBuffer.h"
#include "krit/render/DrawCall.h"
#include "krit/render/DrawKey.h"
#include "krit/render/FrameBuffer.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include "krit/render/RenderContext.h"
#include "krit/render/SceneShader.h"
#include "krit/render/Shader.h"
#include "krit/render/SmoothingMode.h"
#include "krit/render/Uniform.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"
#include <SDL2/SDL_error.h>
#include <algorithm>
#include <cassert>
#include <memory>
#include <stdint.h>
#include <utility>
#if TRACY_ENABLE
#include "krit/tracy/Tracy.hpp"
// #include "krit/tracy/TracyOpenGL.hpp"
#endif

namespace krit {

namespace {

SpriteShader *defaultTextureSpriteShader;
SpriteShader *defaultColorSpriteShader;
SpriteShader *defaultTextSpriteShader;

}

void Renderer::setSmoothingMode(SmoothingMode mode, ImageData *img) {
    if (currentRenderTarget && !currentRenderTarget->allowSmoothing) {
        mode = SmoothNearest;
    }
    switch (mode) {
        case SmoothNearest: {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        }
        case SmoothLinear: {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        }
        case SmoothMipmap: {
            if (img && !img->hasMipmaps) {
                img->hasMipmaps = true;
                glGenerateMipmap(GL_TEXTURE_2D);
                checkForGlErrors("generate mipmaps");
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        }
    }
}

void Renderer::setBlendMode(BlendMode mode) {
    switch (mode) {
        case Add: {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
            break;
        }
        case Multiply: {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO,
                                GL_ONE);
            break;
        }
        case BlendScreen: {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ZERO,
                                GL_ONE);
            break;
        }
        case Subtract: {
            glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
            break;
        }
        case Alpha: {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
        }
        case Replace: {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ZERO);
            break;
        }
        default: {
            assert(false);
        }
    }
    checkForGlErrors("setBlendMode");
}

SpriteShader *Renderer::getDefaultTextureShader() {
    if (!defaultTextureSpriteShader) {
        defaultTextureSpriteShader = new SpriteShader(new Shader(
#include "./renderer.texture.vert"
            ,
#include "./renderer.texture.frag"
            ));
    }
    return defaultTextureSpriteShader;
}

SpriteShader *Renderer::getDefaultColorShader() {
    if (!defaultColorSpriteShader) {
        defaultColorSpriteShader = new SpriteShader(new Shader(
#include "./renderer.color.vert"
            ,
#include "./renderer.color.frag"
            ));
    }
    return defaultColorSpriteShader;
}

SpriteShader *Renderer::getDefaultTextShader() {
    if (!defaultTextSpriteShader) {
        defaultTextSpriteShader = new SpriteShader(new Shader(
#include "./renderer.text.vert"
            ,
#include "./renderer.text.frag"
            ));
    }
    return defaultTextSpriteShader;
}

// static void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id,
//                                        GLenum severity, GLsizei length,
//                                        const GLchar *message,
//                                        const void *userParam) {
//     fprintf(stderr,
//             "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
//             (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
//             severity, message);
// }

Matrix4 _ortho;

static RenderFloat _vertices[] = {
    -1.0, -1.0, 0.0, 1.0,  0.0,  0.0, 0.0, 0.0,  0.0, 0.0, 0.0,  0.0, 0.0, 0.0,
    0.0,  0.0,  1.0, -1.0, 0.0,  1.0, 1.0, 0.0,  0.0, 0.0, 0.0,  0.0, 0.0, 0.0,
    0.0,  0.0,  0.0, 0.0,  -1.0, 1.0, 0.0, 1.0,  0.0, 1.0, 0.0,  0.0, 0.0, 0.0,
    0.0,  0.0,  0.0, 0.0,  0.0,  0.0, 1.0, -1.0, 0.0, 1.0, 1.0,  0.0, 0.0, 0.0,
    0.0,  0.0,  0.0, 0.0,  0.0,  0.0, 0.0, 0.0,  1.0, 1.0, 0.0,  1.0, 1.0, 1.0,
    0.0,  0.0,  0.0, 0.0,  0.0,  0.0, 0.0, 0.0,  0.0, 0.0, -1.0, 1.0, 0.0, 1.0,
    0.0,  1.0,  0.0, 0.0,  0.0,  0.0, 0.0, 0.0,  0.0, 0.0, 0.0,  0.0,
};

Renderer::Renderer(Window &_window) : window(_window) {
    SDL_Window *window = _window.window;

    this->glContext = SDL_GL_CreateContext(window);
    if (!this->glContext) {
        panic(SDL_GetError());
    }
    SDL_GL_MakeCurrent(window, this->glContext);
    checkForGlErrors("context");
    SDL_GL_SetSwapInterval(1);
    checkForGlErrors("swap interval");
    glEnable(GL_BLEND);
    checkForGlErrors("blend");
    glDisable(GL_DEPTH_TEST);
    checkForGlErrors("depth test");

#if KRIT_ENABLE_MULTISAMPLING
    glEnable(GL_MULTISAMPLE);
    checkForGlErrors("multisample");
#endif

#if KRIT_USE_GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        panic("%s\n", glewGetErrorString(err));
    }
    checkForGlErrors("glew init");
#endif

    // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    // glDebugMessageCallback(messageCallback, 0);

#if KRIT_ENABLE_TOOLS
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(nullptr);
    checkForGlErrors("imgui init");

    auto &io = ImGui::GetIO();

    unsigned char *pixels = nullptr;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    glActiveTexture(GL_TEXTURE0);
    checkForGlErrors("imgui active texture");
    glGenTextures(1, &Editor::imguiTextureId);
    checkForGlErrors("imgui gen textures");
    glBindTexture(GL_TEXTURE_2D, Editor::imguiTextureId);
    checkForGlErrors("imgui bind texture");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);
    checkForGlErrors("imgui texImage2D");
    glBindTexture(GL_TEXTURE_2D, 0);

    // ImGui::StyleColorsClassic();

    io.Fonts->TexID = (void *)(intptr_t)Editor::imguiTextureId;
    Editor::imguiInitialized = true;
#endif

#ifndef __EMSCRIPTEN__
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);
#endif
    glGenBuffers(3, this->vertexBuffer);
    glGenBuffers(1, &this->sceneShaderVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->sceneShaderVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RenderFloat[96]), _vertices,
                 GL_STATIC_DRAW);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawCommandBuffer.defaultTextureShader = getDefaultTextureShader();
    drawCommandBuffer.defaultColorShader = getDefaultColorShader();

    glDepthRange(-1000, 1000);

    checkForGlErrors("renderer init");

    // #if TRACY_ENABLE
    //     TracyGpuContext;
    // #endif
}

Renderer::~Renderer() {
    if (this->glContext) {
        SDL_GL_DeleteContext(this->glContext);
    }
}

template <>
void Renderer::drawCall<PushClipRect, Rectangle>(RenderContext &ctx,
                                                 Rectangle &clipRect) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::drawCall<PushClipRect>");
#endif
    clipStack.emplace_back();
    Rectangle &newClip = clipStack.back();
    if (clipStack.size() > 1) {
        newClip.copyFrom(clipRect.overlap(clipStack[clipStack.size() - 2]));
    } else {
        newClip.copyFrom(clipRect);
    }
    if (clipStack.size() == 1) {
        glEnable(GL_SCISSOR_TEST);
        checkForGlErrors("enable scissor test");
    }
    updateClip(ctx);
    checkForGlErrors("push clip rect: %i,%i %ix%i", clipRect.x, clipRect.y, clipRect.width, clipRect.height);
}

// template <>
// void Renderer::drawCall<PushDynamicClipRect, Rectangle *>(
//     RenderContext &ctx, Rectangle *&clipRect) {
// #if TRACY_ENABLE
//     ZoneScopedN("Renderer::drawCall<PushClipRect>");
// #endif
//     Vec4f ul = Vec4f(clipRect->x, clipRect->y, 0, 1);
//     Vec4f ur = Vec4f(clipRect->x + clipRect->height, clipRect->y, 0, 1);
//     Vec4f ll = Vec4f(clipRect->x, clipRect->y + clipRect->height, 0, 1);
//     Vec4f lr = Vec4f(clipRect->x + clipRect->width,
//                      clipRect->y + clipRect->height, 0, 1);
//     Matrix4 m;
//     m.identity();
//     ctx.camera->getTransformationMatrix(m, width, height);
//     ul = m * ul;
//     ur = m * ur;
//     ll = m * ll;
//     lr = m * lr;
//     ul.x() = (ul.x() / ul.w() + 1.0) / 2.0 * width;
//     ul.y() = (1.0 - ul.y() / ul.w()) / 2.0 * height;
//     ur.x() = (ur.x() / ur.w() + 1.0) / 2.0 * width;
//     ur.y() = (1.0 - ur.y() / ur.w()) / 2.0 * height;
//     ll.x() = (ll.x() / ll.w() + 1.0) / 2.0 * width;
//     ll.y() = (1.0 - ll.y() / ll.w()) / 2.0 * height;
//     lr.x() = (lr.x() / lr.w() + 1.0) / 2.0 * width;
//     lr.y() = (1.0 - lr.y() / lr.w()) / 2.0 * height;

//     float left = std::min({ul.x(), ur.x(), ll.x(), lr.x()});
//     float width = std::max({ul.x(), ur.x(), ll.x(), lr.x()}) - left;
//     float top = std::min({ul.y(), ur.y(), ll.y(), lr.y()});
//     float height = std::max({ul.y(), ur.y(), ll.y(), lr.y()}) - top;

//     Rectangle clip(left, top, width, height);
//     clipStack.emplace_back();
//     Rectangle &newClip = clipStack.back();
//     if (clipStack.size() > 1) {
//         newClip.copyFrom(clip.overlap(clipStack[clipStack.size() - 2]));
//     } else {
//         newClip.copyFrom(clip);
//     }
//     newClip.width = std::max(newClip.width, 0.0f);
//     newClip.height = std::max(newClip.height, 0.0f);
//     if (clipStack.size() == 1) {
//         glEnable(GL_SCISSOR_TEST);
//     }
//     checkForGlErrors("WTF");
//     glScissor(newClip.x, this->height - newClip.y - newClip.height,
//               newClip.width, newClip.height);
//     checkForGlErrors("push dynamic clip rect");
// }

template <>
void Renderer::drawCall<PopClipRect, char>(RenderContext &ctx, char &_) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::drawCall<PopClipRect>");
#endif
    clipStack.pop_back();
    if (clipStack.empty()) {
        glDisable(GL_SCISSOR_TEST);
    } else {
        updateClip(ctx);
    }
    checkForGlErrors("pop clip rect");
}

template <>
void Renderer::drawCall<SetRenderTarget, SetRenderTargetArgs>(
    RenderContext &ctx, SetRenderTargetArgs &args) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::drawCall<SetRenderTarget>");
#endif
// printf("RENDER TARGET: %i\n", fb ? fb->frameBuffer : 0);
#if KRIT_ENABLE_MULTISAMPLING
// if (args.clear && args.target && args.target->resolvedTexture) {
//     glBindTexture(GL_TEXTURE_2D, args.target->resolvedTexture);
//     glDisable(GL_SCISSOR_TEST);
//     glClearColor(0, 0, 0, 0);
//     glClear(GL_COLOR_BUFFER_BIT);
//     glEnable(GL_SCISSOR_TEST);
//     glBindTexture(GL_TEXTURE_2D, 0);
// }
#endif
    if (args.target) {
        args.target->_markDirty();
    }
    glBindFramebuffer(GL_FRAMEBUFFER,
                      args.target ? args.target->getFramebuffer() : 0);
    checkForGlErrors("bind framebuffer");
    if (args.clear && args.target) {
        if (!clipStack.empty()) {
            glDisable(GL_SCISSOR_TEST);
        }
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        if (!clipStack.empty()) {
            glEnable(GL_SCISSOR_TEST);
        }
        checkForGlErrors("clear");
    }
    currentRenderTarget = args.target;
    this->setSize(ctx);
    updateClip(ctx);
    checkForGlErrors("set size");
}

template <>
void Renderer::drawCall<ClearColor, Color>(RenderContext &ctx, Color &c) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::drawCall<ClearColor>");
#endif
    if (!clipStack.empty()) {
        glDisable(GL_SCISSOR_TEST);
    }
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
    if (!clipStack.empty()) {
        glEnable(GL_SCISSOR_TEST);
    }
}

template <>
void Renderer::drawCall<RenderImGui, ImDrawData *>(RenderContext &ctx,
                                                   ImDrawData *&drawData) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::drawCall<RenderImGui>");
#endif
#if KRIT_ENABLE_TOOLS
    if (drawData) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(Editor::window);
        ImGui_ImplOpenGL3_RenderDrawData(drawData);
    }
#endif
}

template <>
void Renderer::drawCall<DrawTriangles, DrawCall>(RenderContext &ctx,
                                                 DrawCall &drawCall) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::drawCall<DrawTriangles>");
#endif
    // puts("draw");
    checkForGlErrors("drawCall");

    int index = engine->updateCtx().tickId % 3;

    setSize(ctx);

    if (drawCall.length() &&
        (!drawCall.key.image || drawCall.key.image->texture)) {
        // this->triangleCount += drawCall.length();

        if (width > 0 && height > 0) {
            SpriteShader *shader = drawCall.key.shader;
            if (!shader) {
                shader = drawCall.key.image ? getDefaultTextureShader()
                                            : getDefaultColorShader();
            }

            glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer[index]);
            checkForGlErrors("bind buffer");

            shader->bind(ctx);
            checkForGlErrors("bind shader");

            if (drawCall.key.image) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, drawCall.key.image->texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                GL_CLAMP_TO_EDGE);
                checkForGlErrors("bind texture");
            }
            setSmoothingMode(drawCall.key.smooth, drawCall.key.image.get());
            if (shader->matrixIndex > -1) {
                glUniformMatrix4fv(shader->matrixIndex, 1, GL_FALSE,
                                   _ortho.data());
            }
            setBlendMode(drawCall.key.blend);

            glDrawElements(GL_TRIANGLES, drawCall.indices.size(),
                           GL_UNSIGNED_INT, drawCall.indices.data());
            checkForGlErrors("drawElements");

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            shader->unbind();
            if (drawCall.key.image) {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }
}

template <>
void Renderer::drawCall<DrawSceneShader, SceneShader *>(RenderContext &ctx,
                                                        SceneShader *&shader) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::drawCall<DrawSceneShader>");
#endif
    setSize(ctx, true);
    setBlendMode(shader->blend);
    setSmoothingMode(SmoothLinear, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, this->sceneShaderVertexBuffer);
    checkForGlErrors("bindBuffer");

    shader->bind(ctx);
    if (shader->matrixIndex > -1) {
        glUniformMatrix4fv(shader->matrixIndex, 1, GL_FALSE, _ortho.data());
    }
    checkForGlErrors("bind");

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkForGlErrors("drawArrays");

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    shader->unbind();
}

void Renderer::startFrame(RenderContext &ctx) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::startFrame");
#endif
    clear(ctx);
    checkForGlErrors("start frame");
}

void Renderer::renderFrame(RenderContext &ctx) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::renderFrame");
#endif

    setSize(ctx);
    int index = engine->updateCtx().tickId % 3;

    // upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer[index]);
    checkForGlErrors("glBindBuffer");

    if (vertexCapacity < drawCommandBuffer.triangles.capacity()) {
        glBufferData(GL_ARRAY_BUFFER,
                     drawCommandBuffer.triangles.capacity() *
                         sizeof(RenderFloat),
                     drawCommandBuffer.triangles.data(), GL_DYNAMIC_DRAW);
        vertexCapacity = drawCommandBuffer.triangles.capacity();
        checkForGlErrors("glBufferData");
    } else if (!drawCommandBuffer.triangles.empty()) {
        glBufferData(GL_ARRAY_BUFFER,
                     drawCommandBuffer.triangles.capacity() *
                         sizeof(RenderFloat),
                     nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        drawCommandBuffer.triangles.size() *
                            sizeof(RenderFloat),
                        drawCommandBuffer.triangles.data());
        checkForGlErrors("glBufferSubData");
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    dispatchCommands(ctx);
    // printf("triangles: %i\n", this->triangleCount);

    if (ctx.camera->currentDimensions != ctx.camera->dimensions) {
        // TODO
    }
}

void Renderer::clear(RenderContext &ctx) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    auto &bgColor = engine->bgColor;
    glScissor(0, 0, engine->window.x(), engine->window.y());
    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
    // this->triangleCount = 0;
}

void Renderer::dispatchCommands(RenderContext &ctx) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::dispatchCommands");
#endif
    size_t indices[DrawCommandTypeCount] = {0};
    for (auto commandType : this->drawCommandBuffer.buf.commandTypes) {
        size_t index = indices[commandType]++;
        switch (commandType) {
#define DISPATCH_COMMAND(cmd)                                                  \
    case cmd:                                                                  \
        this->drawCall<cmd>(ctx,                                               \
                            this->drawCommandBuffer.buf.get<cmd>()[index]);    \
        break;

            DISPATCH_COMMAND(DrawTriangles)
            DISPATCH_COMMAND(PushClipRect)
            DISPATCH_COMMAND(PopClipRect)
            DISPATCH_COMMAND(SetRenderTarget)
            DISPATCH_COMMAND(DrawSceneShader)
            DISPATCH_COMMAND(ClearColor)
            DISPATCH_COMMAND(RenderImGui)
        }
    }
#undef DISPATCH_COMMAND

    this->drawCommandBuffer.clear();
}

void Renderer::flip(RenderContext &ctx) {
    SDL_GL_SwapWindow(engine->window.window);

    // #if TRACY_ENABLE
    //     TracyGpuCollect;
    // #endif
}

void Renderer::setSize(RenderContext &ctx, bool sceneShader) {
#if TRACY_ENABLE
    ZoneScopedN("Renderer::setSize");
#endif
    IntDimensions size =
        currentRenderTarget ? currentRenderTarget->size : ctx.size();
    Vec2f scale =
        currentRenderTarget ? currentRenderTarget->scale : Vec2f(1, 1);
    width = size.x() * scale.x();
    height = size.y() * scale.y();

    _ortho.identity();
    if (!sceneShader &&
        (!currentRenderTarget || currentRenderTarget->cameraTransform)) {
        ctx.camera->getTransformationMatrix(_ortho, width, height);
    } else if (currentRenderTarget && !currentRenderTarget->cameraTransform) {
        // framebuffer, no camera transform
        _ortho.translate(-width / 2.0, -height / 2.0);
        _ortho.scale(2.0 / width, 2.0 / height, 1.0 / 2000);
        Matrix4 M;
        M.identity();
        M[11] = 1.0;
        _ortho *= M;
    } else {
        // scene shader
        _ortho.translate(-width / 2.0, -height / 2.0);
        _ortho.scale(2.0 / width, -2.0 / height, 1.0 / 2000);
        Matrix4 M;
        M.identity();
        M[11] = 1.0;
        _ortho *= M;
    }

    if (currentRenderTarget) {
        glViewport(0, 0, width / scale.x(), height / scale.y());
    } else {
        glViewport(ctx.camera->offset.x(), ctx.camera->offset.y(), width,
                   height);
    }

    // _ortho.translate(1.0, 1.0);
    // _ortho.scale(width / w, height / h);
    // _ortho.translate(-1.0, -1.0);
}

void Renderer::updateClip(RenderContext &ctx) {
    if (clipStack.empty()) {
        return;
    }
    auto &newClip = clipStack.back();
    int ox = 0, oy = 0;
    if (!currentRenderTarget) {
        ox = ctx.camera->offset.x();
        oy = ctx.camera->offset.y();
    }
    glScissor(newClip.x + ox, this->height - newClip.y - newClip.height + oy,
              newClip.width, newClip.height);
}

}
