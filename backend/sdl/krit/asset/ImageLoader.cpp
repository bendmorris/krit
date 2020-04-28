#include "krit/asset/ImageLoader.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "GLES3/gl3.h"
#include "krit/render/ImageData.h"
#include "krit/render/Renderer.h"
#include "krit/utils/Panic.h"
#include <cstdint>
#include <memory>
#include <string>

shared_ptr<void> ImageLoader::loadAsset(const std::string &id) {
    SDL_LockMutex(Renderer::renderMutex);
    checkForGlErrors("active texture");
    // shortcut for now: just load from file
    SDL_Surface *surface = IMG_Load(id.c_str());
    if (!surface) {
        panic("IMG_Load(%s) is null\n", id.c_str());
    }
    SDL_LockSurface(surface);
    int size = surface->w * surface->h;
    uint8_t *pixels = static_cast<uint8_t*>(surface->pixels);
    double f = 0;
    unsigned int mode = GL_RGB;
    if (surface->format->BytesPerPixel == 4) {
        // premultiply alpha
        for (int i = 0; i < size; ++i) {
            uint8_t a = pixels[i * 4 + 3];
            pixels[i * 4] = (static_cast<uint32_t>(pixels[i * 4]) * a / 0xff);
            pixels[i * 4 + 1] = (static_cast<uint32_t>(pixels[i * 4 + 1]) * a / 0xff);
            pixels[i * 4 + 2] = (static_cast<uint32_t>(pixels[i * 4 + 2]) * a / 0xff);
            f += pixels[i * 4];
        }
        mode = GL_RGBA;
    }
    SDL_UnlockSurface(surface);
    shared_ptr<ImageData> img = make_shared<ImageData>();
    img->dimensions.setTo(surface->w, surface->h);
    // upload texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &img->texture);
    checkForGlErrors("gen textures");
    glBindTexture(GL_TEXTURE_2D, img->texture);
    checkForGlErrors("bind texture");
    glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);
    checkForGlErrors("texImage2D");
    glGenerateMipmap(GL_TEXTURE_2D);
    checkForGlErrors("asset load");
    SDL_FreeSurface(surface);
    SDL_UnlockMutex(Renderer::renderMutex);
    return img;
}
