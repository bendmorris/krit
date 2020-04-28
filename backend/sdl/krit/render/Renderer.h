#ifndef KRIT_RENDER_RENDERER
#define KRIT_RENDER_RENDERER

#include "krit/render/Gl.h"
#include "krit/render/DrawCall.h"
#include "krit/render/DrawCommand.h"
#include "krit/render/BlendMode.h"
#include "krit/render/RenderContext.h"
#include "SDL2/SDL.h"

using namespace krit;

namespace krit {

struct Renderer {
    static SDL_mutex *renderMutex;

    DrawCommandBuffer drawCommandBuffer;

    Renderer();

    void init();
    void startFrame(RenderContext &ctx);
    void flushFrame(RenderContext &ctx);
    void flushBatch(RenderContext &ctx);

    private:
        std::vector<char> renderData;
        GLuint vao;
        GLuint renderBuffer[2];
        GLuint materialBuffer;
        RenderFloat *bufferPtr = nullptr;
        unsigned int triangleCount;
        bool initialized = false;

        int width = 0;
        int height = 0;

        template <size_t, typename T> void drawCall(T&);
};

}

#endif
