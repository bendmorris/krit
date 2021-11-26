#include "krit/render/FrameBuffer.h"

namespace krit {

void FrameBuffer::init() {
    bool initialized = false;
    if (!frameBuffer) {
        glGenFramebuffers(1, &frameBuffer);
        checkForGlErrors("create framebuffer");
        initialized = true;
    }
    #if KRIT_ENABLE_MULTISAMPLING
    if (multisample && !resolvedFb) {
        glGenFramebuffers(1, &resolvedFb);
        checkForGlErrors("create multisample framebuffer");
        if (initialized) {
            _currentSize.setTo(-1, -1);
        }
        initialized = true;
    }
    #endif
    if (initialized) {
        this->resize(size.width(), size.height());
    }
}

void FrameBuffer::resize(unsigned int width, unsigned int height) {
    size.setTo(width, height);
    this->_resize();
}

void FrameBuffer::createTextures(unsigned int width, unsigned int height) {
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);

    checkForGlErrors("create framebuffer texture: %i %ix%i\n", texture, width,
                     height);

    #if KRIT_ENABLE_MULTISAMPLING
    if (multisample) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, width,
                                height, GL_TRUE);
        checkForGlErrors("glTexImage2DMultisample");

        checkForGlErrors("glTexParameteri");

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D_MULTISAMPLE, texture, 0);
        checkForGlErrors("glFrambufferTexture2D %i", texture);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        checkForGlErrors("glBindTexture");

        {
            GLuint &texture = resolvedTexture;
            glBindFramebuffer(GL_FRAMEBUFFER, resolvedFb);
            glGenTextures(1, &texture);

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                texture, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            image.texture = texture;
        }
    } else
    #endif
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                            texture, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        image.texture = texture;
    }

    image.dimensions.setTo(width, height);
    checkForGlErrors("FrameBuffer::createTextures");
}

ImageData &FrameBuffer::getTexture() {
    #if KRIT_ENABLE_MULTISAMPLING
    if (multisample) {
        GLint drawFboId = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFb);
        checkForGlErrors("glBindFramebuffer");
        glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y,
                        GL_COLOR_BUFFER_BIT, GL_NEAREST);
        checkForGlErrors("glBlitFramebuffer %i -> %i", frameBuffer, resolvedFb);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId);
    }
    #endif
    return image;
}

void FrameBuffer::_resize() {
    init();
    int width = size.width(), height = size.height();
    if (_currentSize.width() != width || _currentSize.height() != height) {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

        if (texture) {
            glDeleteTextures(1, &texture);
        }
        if (resolvedTexture) {
            glDeleteTextures(1, &resolvedTexture);
        }

        _currentSize.setTo(width, height);
        createTextures(width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    checkForGlErrors("resize framebuffer");
}

}
