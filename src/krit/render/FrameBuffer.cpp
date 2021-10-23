#include "krit/render/FrameBuffer.h"

namespace krit {

void BaseFrameBuffer::init() {
    if (!frameBuffer) {
        glGenFramebuffers(1, &frameBuffer);
        checkForGlErrors("create framebuffer");
        this->resize(size.width(), size.height());
    }
}

void BaseFrameBuffer::resize(unsigned int width, unsigned int height) {
    size.setTo(width, height);
    this->_resize();
}

}
