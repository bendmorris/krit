#ifndef KRIT_RENDER_FRAMEBUFFER
#define KRIT_RENDER_FRAMEBUFFER

#include "krit/math/ScaleFactor.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include <cstddef>

namespace krit {

struct FrameBuffer {
    static const int BUFFER_COUNT = 2;

    ScaleFactor scale;
    IntDimensions size;
    bool multisample = false;
    bool doubleBuffer = false;

    FrameBuffer(unsigned int width, unsigned int height,
                bool multisample = false)
        : size(width, height), multisample(multisample) {}

    virtual ~FrameBuffer() {
        for (int i = 0; i < BUFFER_COUNT; ++i) {
            if (texture[i]) {
                glDeleteTextures(1, &texture[i]);
            }
            if (resolvedTexture[i]) {
                glDeleteTextures(1, &resolvedTexture[i]);
            }
            if (frameBuffer[i]) {
                glDeleteFramebuffers(1, &frameBuffer[i]);
            }
            if (resolvedFb[i]) {
                glDeleteFramebuffers(1, &resolvedFb[i]);
            }
        }
    }

    void init();
    void resize(unsigned int width, unsigned int height);
    int index();

    void createTextures(unsigned int width, unsigned int height);
    GLuint getFramebuffer();
    GLuint getTexture();

    friend struct Renderer;
    friend struct ShaderInstance;

private:
    IntDimensions _currentSize[BUFFER_COUNT];
    bool dirty[BUFFER_COUNT] = {0};
    GLuint frameBuffer[BUFFER_COUNT] = {0};
    GLuint texture[BUFFER_COUNT] = {0};
    GLuint resolvedFb[BUFFER_COUNT] = {0};
    GLuint resolvedTexture[BUFFER_COUNT] = {0};

    void _resize();
    void _markDirty();
};

}

#endif
