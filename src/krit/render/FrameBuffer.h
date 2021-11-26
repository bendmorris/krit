#ifndef KRIT_RENDER_FRAMEBUFFER
#define KRIT_RENDER_FRAMEBUFFER

#include "krit/math/ScaleFactor.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include <cstddef>

namespace krit {

struct FrameBuffer {
    GLuint frameBuffer = 0;
    ScaleFactor scale;
    IntDimensions size;
    bool multisample = false;

    FrameBuffer(unsigned int width, unsigned int height,
                bool multisample = false)
        : size(width, height), multisample(multisample) {}

    virtual ~FrameBuffer() {
        if (texture) {
            glDeleteTextures(1, &texture);
        }
        if (resolvedTexture) {
            glDeleteTextures(1, &resolvedTexture);
        }
        if (frameBuffer) {
            glDeleteFramebuffers(1, &frameBuffer);
        }
        if (resolvedFb) {
            glDeleteFramebuffers(1, &resolvedFb);
        }
    }

    void init();
    void resize(unsigned int width, unsigned int height);

    void createTextures(unsigned int width, unsigned int height);

    friend struct Renderer;
    friend struct ShaderInstance;

private:
    GLuint texture = 0;
    ImageData image;
    IntDimensions _currentSize;
    bool _cleared = false;
    GLuint resolvedFb = 0;
    GLuint resolvedTexture = 0;

    ImageData &getTexture();

    void _resize();
};

}

#endif
