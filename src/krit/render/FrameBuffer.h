#ifndef KRIT_RENDER_FRAMEBUFFER
#define KRIT_RENDER_FRAMEBUFFER

#include "krit/math/ScaleFactor.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include <cstddef>

namespace krit {

struct BaseFrameBuffer {
    GLuint frameBuffer = 0;
    ScaleFactor scale;
    IntDimensions size;

    BaseFrameBuffer(int width, int height) : size(width, height) {}
    virtual ~BaseFrameBuffer() {}

    void init();
    void resize(unsigned int width, unsigned int height);

    virtual ImageData *getTexture(int index = 0) { return nullptr; }

private:
    IntDimensions _currentSize;
    virtual void _resize() {
        _currentSize.setTo(size.x, size.y);
    };

    template <size_t N> friend struct FrameBuffer;
};

template <size_t N> struct FrameBuffer : public BaseFrameBuffer {
    GLuint textures[N] = {0};
    ImageData images[N];

    FrameBuffer(unsigned int width, unsigned int height)
        : BaseFrameBuffer(width, height) {}

    ~FrameBuffer() {
        if (textures[0]) {
            glDeleteTextures(N, textures);
        }
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
            images[i].texture = textures[i];
            images[i].dimensions.setTo(width, height);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        checkForGlErrors("create framebuffer textures");
    }

    ImageData *getTexture(int index = 0) override { return &images[index]; }

private:
    void _resize() override {
        init();
        int width = size.width(), height = size.height();
        if (_currentSize.width() != width || _currentSize.height() != height) {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

            if (textures[0]) {
                glDeleteTextures(N, textures);
            }

            _currentSize.setTo(width, height);
            createTextures(width, height);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        checkForGlErrors("resize framebuffer");
    }
};

using FrameBuffer1 = FrameBuffer<1>;
using FrameBuffer2 = FrameBuffer<2>;
using FrameBuffer3 = FrameBuffer<3>;
using FrameBuffer4 = FrameBuffer<4>;

}

#endif
