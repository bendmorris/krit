#ifndef KRIT_RENDER_IMAGE_DATA
#define KRIT_RENDER_IMAGE_DATA

#include "krit/Math.h"
#include "krit/render/Gl.h"

using namespace krit;

namespace krit {

/**
 * Contains backend-specific data for an image.
 */
struct ImageData {
    GLuint texture = 0;
    IntDimensions dimensions;

    ImageData(GLuint texture, IntDimensions dimensions): texture(texture), dimensions(dimensions) {}

    int width() { return this->dimensions.width(); }
    int height() { return this->dimensions.height(); }

    ImageData() {}
};

}

#endif
