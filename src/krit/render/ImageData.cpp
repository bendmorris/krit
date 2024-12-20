#include "krit/render/ImageData.h"
#include "krit/Engine.h"
#include "krit/TaskManager.h"
#include "krit/render/Gl.h"

namespace krit {

ImageData::ImageData(uint8_t *data, size_t width, size_t height)
    : dimensions(width, height) {
    TaskManager::instance->pushRender([this, data]() {
        int width = this->dimensions.x(), height = this->dimensions.y();
        LOG_DEBUG("callback: create image data (%ix%i)", width, height);
        // upload texture
        GLuint texture;
        glActiveTexture(GL_TEXTURE0);
        checkForGlErrors("active texture");
        glGenTextures(1, &texture);
        if (!texture) {
            LOG_ERROR("failed to generate empty texture for new ImageData");
        }
        checkForGlErrors("gen textures");
        glBindTexture(GL_TEXTURE_2D, texture);
        checkForGlErrors("bind texture");
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        checkForGlErrors("texImage2D");
        glBindTexture(GL_TEXTURE_2D, 0);
        this->texture = texture;
        delete[] data;
    });
}

ImageData::~ImageData() {
    if (engine && engine->running && texture && owned) {
        GLuint tex = this->texture;
        TaskManager::instance->pushRender([tex]() {
            LOG_DEBUG("callback: destroy image data");
            glDeleteTextures(1, &tex);
        });
    }
}

}
