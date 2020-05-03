#include "krit/asset/ImageLoader.h"
#include "krit/App.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include "krit/render/RenderContext.h"
#include "krit/render/Renderer.h"
#include "krit/utils/Panic.h"
#include "krit/TaskManager.h"
#include <SDL.h>
#include <SDL_image.h>
#include <cstdint>
#include <memory>
#include <string>

namespace krit {

shared_ptr<void> ImageLoader::loadAsset(const AssetInfo &info) {
    shared_ptr<ImageData> img = make_shared<ImageData>();
    img->dimensions.setTo(info.properties.dimensions);

    TaskManager::instance->push([info, img](UpdateContext &) {
        SDL_Surface *surface = IMG_Load(info.path.c_str());
        if (!surface) {
            panic("IMG_Load(%s) is null\n", info.path.c_str());
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

        TaskManager::instance->pushRender([img, surface, mode](RenderContext &render) {
            // upload texture
            SDL_LockMutex(Renderer::renderMutex);
            glActiveTexture(GL_TEXTURE0);
            checkForGlErrors("active texture");
            glGenTextures(1, &img->texture);
            checkForGlErrors("gen textures");
            glBindTexture(GL_TEXTURE_2D, img->texture);
            checkForGlErrors("bind texture");
            glTexImage2D(GL_TEXTURE_2D, 0, mode, img->width(), img->height(), 0, mode, GL_UNSIGNED_BYTE, surface->pixels);
            checkForGlErrors("texImage2D");
            glGenerateMipmap(GL_TEXTURE_2D);
            checkForGlErrors("asset load");
            SDL_UnlockMutex(Renderer::renderMutex);

            TaskManager::instance->push([surface](UpdateContext &) {
                SDL_FreeSurface(surface);
            });
        });
    });

    return img;
}

}
