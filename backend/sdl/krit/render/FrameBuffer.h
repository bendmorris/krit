#ifndef KRIT_RENDER_FRAMEBUFFER
#define KRIT_RENDER_FRAMEBUFFER

#include <cstddef>
#include "GLES3/gl3.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"

namespace krit {

struct BaseFrameBuffer {
    GLuint frameBuffer = 0;
    unsigned int width = 0;
    unsigned int height = 0;

    BaseFrameBuffer() {
        glGenFramebuffers(1, &frameBuffer);
        checkForGlErrors("create framebuffer");
    }

    virtual void resize(unsigned int width, unsigned int height) {}
};

template <size_t N> struct FrameBuffer: public BaseFrameBuffer {
    GLuint textures[N] = {0};

    FrameBuffer(unsigned int width, unsigned int height): BaseFrameBuffer() {
        resize(width, height);
    }

    void resize(unsigned int width, unsigned int height) override {
        if (this->width != width || this->height != height) {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

            if (textures[0]) {
                glDeleteTextures(N, textures);
            }

            this->width = width;
            this->height = height;
            createTextures(width, height);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        checkForGlErrors("resize framebuffer");
    }

    void createTextures(unsigned int width, unsigned int height) {
        glGenTextures(N, textures);

        for (int i = 0; i < N; ++i) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            checkForGlErrors("create texture %i: %i %ix%i\n", i, textures[i], width, height);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER , GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // specify texture as color attachment

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
        }
        checkForGlErrors("create framebuffer textures");
    }

    ImageData getTexture(int index = 0) {
        return ImageData(textures[index], IntDimensions(width, height));
    }
};

}

#endif
