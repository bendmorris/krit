#ifndef KRIT_RENDER_FRAMEBUFFER
#define KRIT_RENDER_FRAMEBUFFER

#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include "krit/math/ScaleFactor.h"
#include <cstddef>

namespace krit {

struct BaseFrameBuffer {
    GLuint frameBuffer = 0;
    ScaleFactor scale;
    IntDimensions currentSize, requestedSize;

    BaseFrameBuffer(int width, int height) : requestedSize(width, height) {}
    virtual ~BaseFrameBuffer() {}

    void init() {
        if (!frameBuffer) {
            glGenFramebuffers(1, &frameBuffer);
            checkForGlErrors("create framebuffer");
            this->resize(requestedSize.width(), requestedSize.height());
        }
    }

    void resize(unsigned int width, unsigned int height) {
        requestedSize.setTo(width, height);
    }

    virtual void _resize() {}
};

template <size_t N> struct FrameBuffer : public BaseFrameBuffer {
    GLuint textures[N] = {0};

    FrameBuffer(unsigned int width, unsigned int height)
        : BaseFrameBuffer(width, height) {}

    ~FrameBuffer() {
        if (textures[0]) {
            glDeleteTextures(N, textures);
        }
    }

    void _resize() override {
        int width = requestedSize.width(), height = requestedSize.height();
        init();
        if (this->currentSize.width() != width ||
            this->currentSize.height() != height) {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

            if (textures[0]) {
                glDeleteTextures(N, textures);
            }

            this->currentSize.setTo(width, height);
            createTextures(width, height);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        checkForGlErrors("resize framebuffer");
    }

    void createTextures(unsigned int width, unsigned int height) {
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(N, textures);

        for (size_t i = 0; i < N; ++i) {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            checkForGlErrors("create texture %i: %i %ix%i\n", i, textures[i],
                             width, height);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // specify texture as color attachment

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                   GL_TEXTURE_2D, textures[i], 0);
        }
        checkForGlErrors("create framebuffer textures");
    }

    ImageData getTexture(int index = 0) {
        _resize();
        return ImageData(textures[index], currentSize);
    }
};

using FrameBuffer1 = FrameBuffer<1>;
using FrameBuffer2 = FrameBuffer<2>;

}

#endif
