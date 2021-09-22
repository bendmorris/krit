#include "krit/render/Renderer.h"
#if ENABLE_TOOLS
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#endif
#include "krit/App.h"
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

namespace krit {

namespace {

SpriteShader *defaultTextureSpriteShader;
SpriteShader *defaultColorSpriteShader;
SpriteShader *defaultTextSpriteShader;

}

void Renderer::setSmoothingMode(SmoothingMode mode) {
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
        default: {
            assert(false);
        }
    }
    checkForGlErrors("setBlendMode");
}

SpriteShader *Renderer::getDefaultTextureShader() {
    if (!defaultTextureSpriteShader) {
        defaultTextureSpriteShader =
            new SpriteShader(new Shader(R"(
// Krit texture vertex shader
attribute vec4 aPosition;
attribute vec2 aTexCoord;
attribute vec4 aColor;
varying vec2 vTexCoord;
varying vec4 vColor;
uniform mat4 uMatrix;

void main(void) {
    vColor = vec4(aColor.rgb * aColor.a, aColor.a);
    vTexCoord = aTexCoord;
    gl_Position = uMatrix * aPosition;
}
)",
                                        R"(// Krit texture fragment shader
varying vec4 vColor;
varying vec2 vTexCoord;
uniform sampler2D uImage;

void main(void) {
    vec4 color = texture2D(uImage, vTexCoord);
    if (color.a == 0.0) {
        discard;
    } else {
        gl_FragColor = color * vColor;
    }
}
)"));
    }
    return defaultTextureSpriteShader;
}

SpriteShader *Renderer::getDefaultColorShader() {
    if (!defaultColorSpriteShader) {
        defaultColorSpriteShader = new SpriteShader(new Shader(R"(
// Krit color vertex shader
attribute vec4 aPosition;
attribute vec4 aColor;
varying vec4 vColor;
uniform mat4 uMatrix;

void main(void) {
    vColor = vec4(aColor.rgb * aColor.a, aColor.a);
    gl_Position = uMatrix * aPosition;
})",
                                                               R"(
// Krit color fragment shader
varying vec4 vColor;

void main(void) {
    gl_FragColor = vColor;
}
)"));
    }
    return defaultColorSpriteShader;
}

SpriteShader *Renderer::getDefaultTextShader() {
    if (!defaultTextSpriteShader) {
        defaultTextSpriteShader =
            new SpriteShader(new Shader(R"(
// Krit text vertex shader
attribute vec4 aPosition;
attribute vec2 aTexCoord;
attribute vec4 aColor;
varying vec2 vTexCoord;
varying vec4 vColor;
uniform mat4 uMatrix;

void main(void) {
    vColor = vec4(aColor.rgb * aColor.a, aColor.a);
    vTexCoord = aTexCoord;
    gl_Position = uMatrix * aPosition;
}
)",
                                        R"(// Krit text fragment shader
varying vec4 vColor;
varying vec2 vTexCoord;
uniform sampler2D uImage;

void main(void) {
    vec4 color = texture2D(uImage, vTexCoord);
    if (color.r == 0.0) {
        discard;
    } else {
        gl_FragColor = vec4(color.r, color.r, color.r, color.r) * vColor;
    }
}
)"));
    }
    return defaultTextSpriteShader;
}

RenderFloat _ortho[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

void ortho(RenderFloat x0, RenderFloat x1, RenderFloat y0, RenderFloat y1) {
    RenderFloat sx = 1.0 / (x1 - x0);
    RenderFloat sy = 1.0 / (y1 - y0);
    _ortho[0] = 2.0 * sx;
    _ortho[5] = 2.0 * sy;
    _ortho[12] = -(x0 + x1) * sx;
    _ortho[13] = -(y0 + y1) * sy;
}

static RenderFloat _vertices[24] = {-1.0, -1.0, 0.0, 0.0, 1.0,  -1.0, 1.0, 0.0,
                                    -1.0, 1.0,  0.0, 1.0, 1.0,  -1.0, 1.0, 0.0,
                                    1.0,  1.0,  1.0, 1.0, -1.0, 1.0,  0.0, 1.0};

Renderer::Renderer() {}

void Renderer::init(SDL_Window *window) {
    if (!initialized) {
        // SDL_GL
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, GL_TRUE);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, GL_TRUE);

        this->glContext = SDL_GL_CreateContext(window);
        if (!this->glContext) {
            panic(SDL_GetError());
        }
        SDL_GL_MakeCurrent(window, this->glContext);
        // // try to get adaptive vsync
        // int result = SDL_GL_SetSwapInterval(-1);
        // // fall back to regular vsync
        // if (result == -1) {
        SDL_GL_SetSwapInterval(1);
        // }
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            panic("%s\n", glewGetErrorString(err));
        }
        checkForGlErrors("glew init");

#if ENABLE_TOOLS
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

        io.Fonts->TexID = (void *)(intptr_t)Editor::imguiTextureId;
        Editor::imguiInitialized = true;
#endif

        glGenVertexArrays(1, &this->vao);
        glBindVertexArray(this->vao);
        glGenBuffers(2, this->renderBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->renderBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(RenderFloat[24]), _vertices,
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        checkForGlErrors("renderer init");
        initialized = true;
    }
}

template <>
void Renderer::drawCall<PushClipRect, Rectangle>(RenderContext &ctx,
                                                 Rectangle &clipRect) {
    clipStack.emplace_back();
    Rectangle &newClip = clipStack.back();
    if (clipStack.size() > 1) {
        newClip.setTo(clipRect.overlap(clipStack[clipStack.size() - 2]));
    } else {
        newClip.setTo(clipRect);
    }
    glScissor(newClip.x, this->height - newClip.y - newClip.height,
              newClip.width, newClip.height);
}

template <>
void Renderer::drawCall<PopClipRect, char>(RenderContext &ctx, char &_) {
    clipStack.pop_back();
    if (clipStack.empty()) {
        glScissor(0, 0, this->width, this->height);
    } else {
        Rectangle &newClip = clipStack.back();
        glScissor(newClip.x, this->height - newClip.y - newClip.height,
                  newClip.width, newClip.height);
    }
}

template <>
void Renderer::drawCall<SetRenderTarget, BaseFrameBuffer *>(
    RenderContext &ctx, BaseFrameBuffer *&fb) {
    // printf("RENDER TARGET: %i\n", fb ? fb->frameBuffer : 0);
    if (fb) {
        fb->_resize();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fb ? fb->frameBuffer : 0);
    checkForGlErrors("bind framebuffer");
}

template <>
void Renderer::drawCall<ClearColor, Color>(RenderContext &ctx, Color &c) {
    glDisable(GL_SCISSOR_TEST);
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
}

template <>
void Renderer::drawCall<RenderImGui, ImDrawData *>(RenderContext &ctx,
                                                   ImDrawData *&drawData) {
#if ENABLE_TOOLS
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
    // puts("draw");
    checkForGlErrors("drawCall");

    if (drawCall.length() &&
        (!drawCall.key.image || drawCall.key.image->texture)) {
        this->triangleCount += drawCall.length();

        if (width > 0 && height > 0) {
            SpriteShader *shader = drawCall.key.shader;
            if (!shader) {
                shader = drawCall.key.image ? getDefaultTextureShader()
                                            : getDefaultColorShader();
            }

            if (drawCall.key.image) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, drawCall.key.image->texture);
                setSmoothingMode(drawCall.key.smooth);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                GL_CLAMP_TO_EDGE);
                checkForGlErrors("bind texture");
            }
            shader->bind(ctx);
            if (shader->matrixIndex > -1) {
                glUniformMatrix4fv(shader->matrixIndex, 1, GL_FALSE, _ortho);
            }
            setBlendMode(drawCall.key.blend);

            int dataSize =
                drawCall.length() * shader->shader.bytesPerVertex * 3;
            this->renderData.reserve(dataSize);
            glBindBuffer(GL_ARRAY_BUFFER, this->renderBuffer[0]);
            checkForGlErrors("bind buffer");
            if ((RenderFloat *)this->renderData.data() != this->bufferPtr) {
                glBufferData(GL_ARRAY_BUFFER, this->renderData.capacity(),
                             (RenderFloat *)this->renderData.data(),
                             GL_DYNAMIC_DRAW);
                checkForGlErrors("buffer data");
                this->bufferPtr = (RenderFloat *)this->renderData.data();
            }
            shader->prepare(ctx, &drawCall,
                            (RenderFloat *)this->renderData.data());
            checkForGlErrors("prepare");

            glDrawArrays(GL_TRIANGLES, 0, drawCall.length() * 3);
            checkForGlErrors("drawArrays");

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            shader->unbind();
        }
    }
}

template <>
void Renderer::drawCall<DrawSceneShader, SceneShader *>(RenderContext &ctx,
                                                        SceneShader *&shader) {
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_SRC_ALPHA, GL_ONE);

    shader->bind(ctx);
    if (shader->matrixIndex > -1) {
        glUniformMatrix4fv(shader->matrixIndex, 1, GL_FALSE, _ortho);
    }
    checkForGlErrors("bind");

    glBindBuffer(GL_ARRAY_BUFFER, this->renderBuffer[1]);
    checkForGlErrors("bindBuffer");

    setBlendMode(shader->blend);

    renderData.reserve(shader->shader.bytesPerVertex * 6);
    shader->prepare(ctx, (RenderFloat *)renderData.data());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkForGlErrors("drawArrays");

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    shader->unbind();
}

void Renderer::renderFrame(RenderContext &ctx) {
    ctx.app->getWindowSize(&this->width, &this->height);
    ortho(0, this->width, this->height, 0);
    glViewport(0, 0, this->width, this->height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    this->triangleCount = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkForGlErrors("start frame");

    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, this->width, this->height);
    checkForGlErrors("start batch");

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

    glDisable(GL_SCISSOR_TEST);

    this->drawCommandBuffer.clear();

    SDL_GL_SwapWindow(ctx.app->window);
    // printf("triangles: %i\n", this->triangleCount);
}

}
