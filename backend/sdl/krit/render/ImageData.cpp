#include "krit/render/ImageData.h"
#include "krit/render/Renderer.h"
#include "krit/TaskManager.h"

using namespace krit;

namespace krit {

ImageData::~ImageData() {
    GLuint texture = this->texture;
    TaskManager::instance->pushRender([texture](RenderContext&) {
        glDeleteTextures(1, &texture);
    });
}

}
