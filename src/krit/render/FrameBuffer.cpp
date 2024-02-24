#include "krit/render/FrameBuffer.h"
#include "krit/Engine.h"
#include "krit/render/Gl.h"
#include <cassert>

namespace krit {

FrameBuffer::~FrameBuffer() {
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
    if (pbo) {
        glDeleteBuffers(1, &pbo);
    }
}

void FrameBuffer::init() {
    bool initialized = false;
    // multisample = false;
    if (!frameBuffer) {
        glGenFramebuffers(1, &frameBuffer);
        checkForGlErrors("create framebuffer");
        initialized = true;
    }
#if KRIT_ENABLE_MULTISAMPLING
    if (multisample && !resolvedFb) {
        glGenFramebuffers(1, &resolvedFb);
        checkForGlErrors("create multisample framebuffer");
        initialized = true;
    }
#endif
    if (initialized) {
        _currentSize.setTo(-1, -1);
        this->resize(size.x(), size.y());
    }
}

void FrameBuffer::resize(unsigned int width, unsigned int height) {
    assert(width && height);
    size.setTo(width, height);
    this->_resize();
}

void FrameBuffer::createTextures(unsigned int width, unsigned int height) {
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    if (!texture) {
        LOG_ERROR("failed to generate texture for FrameBuffer");
    }
    checkForGlErrors("create framebuffer texture: %i %ix%i\n", texture,
                     width, height);

#if KRIT_ENABLE_MULTISAMPLING
    if (multisample) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, format, width,
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
            // glClear(0);
            glGenTextures(1, &texture);
            if (!texture) {
                LOG_ERROR(
                    "failed to generate multisample texture for FrameBuffer");
            }

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                         format, GL_UNSIGNED_BYTE, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, texture, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    } else
#endif
    {
        GLuint &texture = this->texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
                     GL_UNSIGNED_BYTE, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, texture, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    checkForGlErrors("FrameBuffer::createTextures");
}

void FrameBuffer::_resize() {
    init();
    int width = size.x(), height = size.y();
    if (_currentSize.x() != width ||
        _currentSize.y() != height) {
        GLint drawFboId = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        // glClear(0);

        if (texture) {
            glDeleteTextures(1, &texture);
        }
        if (resolvedTexture) {
            glDeleteTextures(1, &resolvedTexture);
        }

        _currentSize.setTo(width, height);
        createTextures(width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
        // glClear(0);
        checkForGlErrors("resize framebuffer");
    }
}

GLuint FrameBuffer::getFramebuffer() {
    init();
    return frameBuffer;
}

GLuint FrameBuffer::getTexture() {
    init();
#if KRIT_ENABLE_MULTISAMPLING
    if (multisample && dirty) {
        dirty = false;
        GLint drawFboId = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFb);
        // glClear(0);
        checkForGlErrors("glBindFramebuffer");
        glBlitFramebuffer(0, 0, size.x(), size.y(), 0, 0, size.x(), size.y(),
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        checkForGlErrors("glBlitFramebuffer %i -> %i", frameBuffer,
                         resolvedFb);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId);
        // glClear(0);
    }
    if (multisample) {
        return resolvedTexture;
    }
#endif
    return texture;
}

void FrameBuffer::_markDirty() { this->dirty = true; }

// uint32_t FrameBuffer::readPixel(int x, int y) {
//     uint8_t pixelData[4];
//     glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
//     glReadPixels(x, size.y() - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
//                  (void *)&pixelData);
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
//     checkForGlErrors("readPixels");
//     Color c(static_cast<float>(pixelData[0]) / 255,
//             static_cast<float>(pixelData[1]) / 255,
//             static_cast<float>(pixelData[2]) / 255,
//             static_cast<float>(pixelData[3]) / 255);
//     return c.rgba();
// }

void FrameBuffer::queueReadPixel(int x, int y) {
    if (!pbo) {
        glGenBuffers(1, &pbo);
        checkForGlErrors("glGenBuffers");
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        checkForGlErrors("glBindBuffer");
        glBufferData(GL_PIXEL_PACK_BUFFER, 4, NULL, GL_STREAM_READ);
        checkForGlErrors("glBufferData");
    } else {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        checkForGlErrors("glBindBuffer");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    checkForGlErrors("glBindFramebuffer");
    glReadPixels(x, size.y() - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    checkForGlErrors("glReadPixels");
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    checkForGlErrors("queueReadPixel");
}

uint32_t FrameBuffer::readPixel() {
    if (!pbo) {
        return 0;
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    checkForGlErrors("glBindBuffer");
    uint8_t *ptr = (uint8_t*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    checkForGlErrors("glMapBuffer");
    assert(ptr);
    Color c(static_cast<float>(ptr[0]) / 255,
            static_cast<float>(ptr[1]) / 255,
            static_cast<float>(ptr[2]) / 255,
            static_cast<float>(ptr[3]) / 255);
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    checkForGlErrors("glUnmapBuffer");
    return c.rgba();
}

std::shared_ptr<ImageData> FrameBuffer::imageData() {
    assert(!doubleBuffer);
    GLuint t = getTexture();
    if (i) {
        i->texture = t;
        i->dimensions.setTo(size.x(), size.y());
    } else {
        i = std::make_shared<ImageData>(t, size);
        i->owned = false;
    }
    return i;
}

}
