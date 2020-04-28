#include "krit/render/ImageData.h"
#include "krit/render/Renderer.h"

using namespace krit;

namespace krit {

ImageData::~ImageData() {
    SDL_LockMutex(Renderer::renderMutex);
    glDeleteTextures(1, &this->texture);
    SDL_UnlockMutex(Renderer::renderMutex);
}

}
