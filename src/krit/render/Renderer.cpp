#include "krit/render/BlendMode.h"
#include "krit/render/DrawCall.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include "krit/render/RenderContext.h"
#include "krit/render/Renderer.h"
#include "krit/render/Shader.h"
#include "krit/App.h"
#include "krit/Math.h"
#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

using namespace krit;

namespace krit {

SDL_mutex *Renderer::renderMutex = SDL_CreateMutex();

RenderFloat _ortho[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

void ortho(RenderFloat x0, RenderFloat x1, RenderFloat y0, RenderFloat y1) {
    RenderFloat sx = 1.0 / (x1 - x0);
    RenderFloat sy = 1.0 / (y1 - y0);
    _ortho[0] = 2.0 * sx;
    _ortho[5] = 2.0 * sy;
    _ortho[12] = -(x0 + x1) * sx;
    _ortho[13] = -(y0 + y1) * sy;
}

void setBlendMode(BlendMode mode) {
    switch (mode) {
        case Add: {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
            break;
        }
        case Multiply: {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
            break;
        }
        case BlendScreen: {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE);
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
    }
    checkForGlErrors("setBlendMode");
}

static RenderFloat _vertices[24] = {
    -1.0, -1.0, 0.0, 0.0,
     1.0, -1.0, 1.0, 0.0,
    -1.0,  1.0, 0.0, 1.0,
     1.0, -1.0, 1.0, 0.0,
     1.0,  1.0, 1.0, 1.0,
    -1.0,  1.0, 0.0, 1.0
};

Renderer::Renderer() {}

void Renderer::init() {
    if (!initialized) {
        glGenVertexArrays(1, &this->vao);
        glBindVertexArray(this->vao);
        glGenBuffers(2, this->renderBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->renderBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(RenderFloat[24]), _vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        checkForGlErrors("renderer init");
        initialized = true;
    }
}

void Renderer::startFrame(RenderContext &ctx) {
    init();
    ctx.app->backend.getWindowSize(&this->width, &this->height);
    ortho(0, this->width, this->height, 0);
    glViewport(0, 0, this->width, this->height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    this->triangleCount = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkForGlErrors("start frame");
}

void Renderer::flushFrame(RenderContext &ctx) {
    SDL_GL_SwapWindow(ctx.app->backend.window);
    // printf("triangles: %i\n", this->triangleCount);
}

template <> void Renderer::drawCall<SetClipRect, Rectangle>(Rectangle &clipRect) {
    glScissor(clipRect.x, this->height - clipRect.y - clipRect.height, clipRect.width, clipRect.height);
}

template <> void Renderer::drawCall<SetRenderTarget, BaseFrameBuffer*>(BaseFrameBuffer *&fb) {
    // printf("RENDER TARGET: %i\n", fb ? fb->frameBuffer : 0);
    glBindFramebuffer(GL_FRAMEBUFFER, fb ? fb->frameBuffer : 0);
    checkForGlErrors("bind framebuffer");
}

template <> void Renderer::drawCall<ClearColor, Color>(Color &c) {
    glDisable(GL_SCISSOR_TEST);
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
}

template <> void Renderer::drawCall<RenderImGui, ImDrawData*>(ImDrawData *&drawData) {
    if (drawData) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(Editor::window);
        ImGui_ImplOpenGL3_RenderDrawData(drawData);
    }
}

template <> void Renderer::drawCall<DrawTriangles, DrawCall>(DrawCall &drawCall) {
    // puts("draw");
    checkForGlErrors("drawCall");

    if (drawCall.length() && (!drawCall.key.image || drawCall.key.image->texture)) {
        this->triangleCount += drawCall.length();

        if (width > 0 && height > 0) {
            SpriteShader *shader = drawCall.key.shader;
            if (!shader) {
                shader = drawCall.key.image ? &SpriteShader::defaultTextureSpriteShader : &SpriteShader::defaultColorSpriteShader;
            }

            if (drawCall.key.image) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, drawCall.key.image->texture);
                switch (drawCall.key.smooth) {
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
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        break;
                    }
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                checkForGlErrors("bind texture");
            }
            shader->bindOrtho(_ortho);
            setBlendMode(drawCall.key.blend);
            checkForGlErrors("set blend mode");

            int dataSize = drawCall.length() * shader->bytesPerVertex() * 3;
            this->renderData.reserve(dataSize);
            glBindBuffer(GL_ARRAY_BUFFER, this->renderBuffer[0]);
            checkForGlErrors("bind buffer");
            if ((RenderFloat *)this->renderData.data() != this->bufferPtr) {
                glBufferData(GL_ARRAY_BUFFER, this->renderData.capacity(), (RenderFloat *)this->renderData.data(), GL_DYNAMIC_DRAW);
                checkForGlErrors("buffer data");
                this->bufferPtr = (RenderFloat *)this->renderData.data();
            }
            shader->prepare(&drawCall, (RenderFloat *)this->renderData.data());
            checkForGlErrors("prepare");

            glDrawArrays(GL_TRIANGLES, 0, drawCall.length() * 3);
            checkForGlErrors("drawArrays");

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            shader->unbind();
        }
    }
}

template <> void Renderer::drawCall<DrawMaterial, Material>(Material &material) {
    // puts("material");
    checkForGlErrors("material");

    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_SRC_ALPHA, GL_ONE);

    Shader *shader = material.shader;
    shader->bind();
    checkForGlErrors("bind");

    glBindBuffer(GL_ARRAY_BUFFER, this->renderBuffer[1]);
    RenderFloat *v = (RenderFloat*)this->renderData.data();
    checkForGlErrors("bindBuffer");
    RenderFloat *origin = nullptr;
    glEnableVertexAttribArray(shader->positionIndex);
    glEnableVertexAttribArray(shader->texCoordIndex);
    glVertexAttribPointer(shader->positionIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(RenderFloat), origin);
    checkForGlErrors("vertex attrib pointers 1");
    glVertexAttribPointer(shader->texCoordIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(RenderFloat), origin + 2);
    checkForGlErrors("vertex attrib pointers 2");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.img.texture);
    checkForGlErrors("bind texture");

    for (auto it : material.uniforms) {
        auto loc = it.first;
        auto &uniform = it.second;
        switch (uniform.type) {
            case UniformInt:
            case UniformTexture: {
                glUniform1i(loc, uniform.intValue);
                break;
            }
            case UniformFloat: {
                glUniform1f(loc, uniform.floatValue);
                break;
            }
            case UniformVec2: {
                glUniform2f(loc, uniform.vec2Value[0], uniform.vec2Value[1]);
                break;
            }
            case UniformVec3: {
                glUniform3f(loc, uniform.vec3Value[0], uniform.vec3Value[1], uniform.vec3Value[2]);
                break;
            }
            case UniformVec4: {
                glUniform4f(loc, uniform.vec4Value[0], uniform.vec4Value[1], uniform.vec4Value[2], uniform.vec4Value[3]);
                break;
            }
            case UniformFloat3v: {
                glUniform3fv(loc, uniform.floatData.first, uniform.floatData.second);
                break;
            }
            case UniformFloat4v: {
                glUniform4fv(loc, uniform.floatData.first, uniform.floatData.second);
                break;
            }
            default: {
                panic("unknown uniform type: %i\n", uniform.type);
            }
        }
    }
    checkForGlErrors("uniforms");

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkForGlErrors("drawArrays");

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(shader->positionIndex);
    glDisableVertexAttribArray(shader->texCoordIndex);

    shader->unbind();
}

void Renderer::flushBatch(RenderContext &ctx) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, this->width, this->height);
    checkForGlErrors("start batch");

    size_t indices[DrawCommandTypeCount] = {0};
    for (auto commandType : this->drawCommandBuffer.commandTypes) {
        size_t index = indices[commandType]++;
        switch (commandType) {
            #define DISPATCH_COMMAND(cmd) case cmd: this->drawCall<cmd>(this->drawCommandBuffer.get<cmd>()[index]); break;

            DISPATCH_COMMAND(DrawTriangles)
            DISPATCH_COMMAND(SetClipRect)
            DISPATCH_COMMAND(SetRenderTarget)
            DISPATCH_COMMAND(DrawMaterial)
            DISPATCH_COMMAND(ClearColor)
            DISPATCH_COMMAND(RenderImGui)
        }
    }

    glDisable(GL_SCISSOR_TEST);

    this->drawCommandBuffer.clear();
}

}
