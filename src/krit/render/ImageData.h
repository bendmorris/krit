#ifndef KRIT_RENDER_IMAGEDATA
#define KRIT_RENDER_IMAGEDATA

#include "krit/Math.h"
#include "krit/render/Gl.h"

namespace krit {

struct Renderer;

/**
 * Contains backend-specific data for an image.
 */
struct ImageData {
    GLuint texture = 0;
    float scale = 1.0;
    IntDimensions dimensions;

    ImageData(GLuint texture, IntDimensions dimensions)
        : texture(texture), dimensions(dimensions) {}

    ImageData(uint8_t *data, size_t width, size_t height);

    int width() { return this->dimensions.width(); }
    int height() { return this->dimensions.height(); }

    ImageData() {}
    ~ImageData();

private:
    bool hasMipmaps = false;

    friend struct Renderer;
};

}

#endif
