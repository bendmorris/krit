#ifndef KRIT_RENDER_RENDERER
#define KRIT_RENDER_RENDERER

#include "krit/Window.h"
#include "krit/math/Rectangle.h"
#include "krit/render/BlendMode.h"
#include "krit/render/DrawCall.h"
#include "krit/render/DrawCommand.h"
#include "krit/render/Gl.h"
#include "krit/render/RenderContext.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_video.h>
#include <stddef.h>
#include <vector>

namespace krit {
struct SpriteShader;
struct FrameBuffer;

struct Renderer {
    SDL_GLContext glContext;

    DrawCommandBuffer drawCommandBuffer;

    Renderer(Window &window);
    ~Renderer();

    void startFrame(RenderContext &ctx);
    void renderFrame(RenderContext &ctx);
    void flip(RenderContext &ctx);

    SpriteShader *getDefaultTextureShader();
    SpriteShader *getDefaultColorShader();
    SpriteShader *getDefaultTextShader();

private:
    GLuint vao;
    GLuint sceneShaderVertexBuffer = 0;
    GLuint vertexBuffer[3] = {0};
    size_t vertexCapacity = 0;
    std::vector<Rectangle> clipStack;
    Window &window;
    FrameBuffer *currentRenderTarget = nullptr;

    int width = 0;
    int height = 0;

    template <size_t, typename T> void drawCall(RenderContext &ctx, T &);
    void setSmoothingMode(SmoothingMode mode, ImageData *img);
    void setBlendMode(BlendMode mode);
    void setSize(RenderContext &ctx, bool sceneShader = false);
    void clear(RenderContext &ctx);
    void updateClip(RenderContext &ctx);
    void dispatchCommands(RenderContext &ctx);
};

}

#endif
