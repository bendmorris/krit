#ifndef KRIT_RENDER_RENDERER
#define KRIT_RENDER_RENDERER

#include "krit/Window.h"
#include "krit/math/Rectangle.h"
#include "krit/render/BlendMode.h"
#include "krit/render/DrawCall.h"
#include "krit/render/DrawCommand.h"
#include "krit/render/Gl.h"
#include "krit/render/RenderContext.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_video.h>
#include <stddef.h>
#include <vector>

namespace krit {
struct SpriteShader;
struct BaseFrameBuffer;

struct Renderer {
    SDL_GLContext glContext;

    DrawCommandBuffer drawCommandBuffer;

    Renderer(Window &window);

    void renderFrame(RenderContext &ctx);

    SpriteShader *getDefaultTextureShader();
    SpriteShader *getDefaultColorShader();
    SpriteShader *getDefaultTextShader();

private:
    std::vector<char> renderData;
    GLuint vao;
    GLuint renderBuffer[2];
    GLuint materialBuffer;
    RenderFloat *bufferPtr = nullptr;
    unsigned int triangleCount;
    std::vector<Rectangle> clipStack;
    Window &window;
    BaseFrameBuffer *currentRenderTarget = nullptr;

    int width = 0;
    int height = 0;

    template <size_t, typename T> void drawCall(RenderContext &ctx, T &);
    void setSmoothingMode(SmoothingMode mode);
    void setBlendMode(BlendMode mode);
    void setSize(RenderContext &ctx);
};

}

#endif
