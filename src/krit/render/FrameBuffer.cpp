#include "krit/render/FrameBuffer.h"
#include "krit/Engine.h"
#include "krit/render/Gl.h"
#include <cassert>

namespace krit {

int FrameBuffer::index() {
    return doubleBuffer
               ? (engine->updateCtx().tickId % FrameBuffer::BUFFER_COUNT)
               : 0;
}

void FrameBuffer::init() {
    bool initialized = false;
    // multisample = false;
    if (!frameBuffer[index()]) {
        glGenFramebuffers(1, &frameBuffer[index()]);
        checkForGlErrors("create framebuffer");
        initialized = true;
    }
#if KRIT_ENABLE_MULTISAMPLING
    if (multisample && !resolvedFb[index()]) {
        glGenFramebuffers(1, &resolvedFb[index()]);
        checkForGlErrors("create multisample framebuffer");
        initialized = true;
    }
#endif
    if (initialized) {
        _currentSize[index()].setTo(-1, -1);
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
    glGenTextures(1, &texture[index()]);
    if (!texture[index()]) {
        LOG_ERROR("failed to generate texture for FrameBuffer");
    }
    checkForGlErrors("create framebuffer texture: %i %ix%i\n", texture[index()],
                     width, height);

#if KRIT_ENABLE_MULTISAMPLING
    if (multisample) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture[index()]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, format, width,
                                height, GL_TRUE);
        checkForGlErrors("glTexImage2DMultisample");

        checkForGlErrors("glTexParameteri");

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D_MULTISAMPLE, texture[index()], 0);
        checkForGlErrors("glFrambufferTexture2D %i", texture[index()]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        checkForGlErrors("glBindTexture");

        {
            GLuint &texture = resolvedTexture[index()];
            glBindFramebuffer(GL_FRAMEBUFFER, resolvedFb[index()]);
            // glClear(0);
            glGenTextures(1, &texture);
            if (!texture) {
                LOG_ERROR(
                    "failed to generate multisample texture for FrameBuffer");
            }

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
    } else
#endif
    {
        GLuint &texture = this->texture[index()];
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
    if (_currentSize[index()].x() != width ||
        _currentSize[index()].y() != height) {
        GLint drawFboId = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[index()]);
        // glClear(0);

        if (texture[index()]) {
            glDeleteTextures(1, &texture[index()]);
        }
        if (resolvedTexture[index()]) {
            glDeleteTextures(1, &resolvedTexture[index()]);
        }

        _currentSize[index()].setTo(width, height);
        createTextures(width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
        // glClear(0);
        checkForGlErrors("resize framebuffer");
    }
}

GLuint FrameBuffer::getFramebuffer() {
    init();
    return frameBuffer[index()];
}

GLuint FrameBuffer::getTexture() {
    init();
#if KRIT_ENABLE_MULTISAMPLING
    if (multisample && dirty[index()]) {
        dirty[index()] = false;
        GLint drawFboId = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer[index()]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFb[index()]);
        // glClear(0);
        checkForGlErrors("glBindFramebuffer");
        glBlitFramebuffer(0, 0, size.x(), size.y(), 0, 0, size.x(), size.y(),
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        checkForGlErrors("glBlitFramebuffer %i -> %i", frameBuffer[index()],
                         resolvedFb[index()]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId);
        // glClear(0);
    }
    if (multisample) {
        return resolvedTexture[index()];
    }
#endif
    return texture[index()];
}

void FrameBuffer::_markDirty() { this->dirty[index()] = true; }

uint32_t FrameBuffer::readPixel(int x, int y) {
    uint8_t pixelData[4];
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[index()]);
    // glClear(0);
    glReadPixels(x, size.y() - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                 (void *)&pixelData);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glClear(0);
    checkForGlErrors("readPixels");
    Color c(static_cast<float>(pixelData[0]) / 255,
            static_cast<float>(pixelData[1]) / 255,
            static_cast<float>(pixelData[2]) / 255,
            static_cast<float>(pixelData[3]) / 255);
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
