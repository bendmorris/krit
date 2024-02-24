#ifndef KRIT_RENDER_FRAMEBUFFER
#define KRIT_RENDER_FRAMEBUFFER

#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include "krit/utils/Pool.h"
#include <cstddef>
#include <memory>

namespace krit {

struct FrameBuffer;

// using FrameBufferPool = Pool<FrameBuffer>;

struct FrameBuffer {
    enum class Quality {
        Low,
        Medium,
        High,
    };

    static FrameBuffer *create(unsigned int width, unsigned int height,
                               bool multisample = false) {
        return new FrameBuffer(width, height, multisample);
    }

    Vec2f scale{1, 1};
    IntDimensions size;
    bool multisample = false;
    bool doubleBuffer = false;
    bool allowSmoothing = true;
    bool cameraTransform = true;
    GLint internalFormat = GL_RGBA;
    GLint format = GL_RGBA;

    FrameBuffer(unsigned int width, unsigned int height,
                bool multisample = false)
        : size(width, height), multisample(multisample) {
        assert(width && height);
    }

    virtual ~FrameBuffer();

    void init();
    void resize(unsigned int width, unsigned int height);

    void createTextures(unsigned int width, unsigned int height);
    GLuint getFramebuffer();
    GLuint getTexture();

    // uint32_t readPixel(int x, int y);

    void queueReadPixel(int x, int y);
    uint32_t readPixel();

    std::shared_ptr<ImageData> imageData();

    friend struct Renderer;
    friend struct ShaderInstance;

private:
    IntDimensions _currentSize;
    bool dirty = false;
    GLuint frameBuffer {0};
    GLuint texture {0};
    GLuint resolvedFb {0};
    GLuint resolvedTexture {0};
    GLuint pbo {0};

    std::shared_ptr<ImageData> i;

    void _resize();
    void _markDirty();
};

}

#endif
