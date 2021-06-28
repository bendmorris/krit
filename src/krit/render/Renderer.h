#ifndef KRIT_RENDER_RENDERER
#define KRIT_RENDER_RENDERER

#include <SDL.h>
#include <GL/glew.h>
#include <stddef.h>
#include <vector>

#include "krit/render/Gl.h"
#include "krit/render/DrawCall.h"
#include "krit/render/DrawCommand.h"
#include "krit/render/BlendMode.h"
#include "krit/render/RenderContext.h"
#include "SDL_mutex.h"
#include "SDL_video.h"
#include "krit/math/Rectangle.h"

namespace krit {
struct RenderContext;
struct SpriteShader;

struct Renderer {
    static SDL_mutex *renderMutex;

    SDL_GLContext glContext;

    DrawCommandBuffer drawCommandBuffer;

    Renderer();

    void init(SDL_Window *window);
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
        bool initialized = false;
        std::vector<Rectangle> clipStack;

        int width = 0;
        int height = 0;

        template <size_t, typename T> void drawCall(T&);
};

}

#endif
