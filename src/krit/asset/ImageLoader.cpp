#include <GL/glew.h>
#include <SDL_image.h>
#include <cstdint>
#include <string>

#include "SDL_pixels.h"
#include "SDL_surface.h"
#include "krit/TaskManager.h"
#include "krit/asset/AssetInfo.h"
#include "krit/asset/AssetLoader.h"
#include "krit/math/Dimensions.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include "krit/utils/Log.h"
#include "krit/utils/Panic.h"

namespace krit {

struct RenderContext;
struct UpdateContext;

template <>
ImageData *AssetLoader<ImageData>::loadAsset(const AssetInfo &info) {
    ImageData *img = new ImageData();
    img->dimensions.setTo(info.properties.dimensions);

    TaskManager::instance->push([info, img](UpdateContext &) {
        SDL_Surface *surface = IMG_Load(info.path.c_str());
        if (!surface) {
            panic("IMG_Load(%s) is null\n", info.path.c_str());
        }
        unsigned int mode = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

        TaskManager::instance->pushRender([info, img, surface,
                                           mode](RenderContext &render) {
            // upload texture
            GLuint texture;
            glActiveTexture(GL_TEXTURE0);
            checkForGlErrors("active texture");
            glGenTextures(1, &texture);
            checkForGlErrors("gen textures");
            glBindTexture(GL_TEXTURE_2D, texture);
            checkForGlErrors("bind texture");
            glTexImage2D(GL_TEXTURE_2D, 0, mode, img->width(), img->height(), 0,
                         mode, GL_UNSIGNED_BYTE, surface->pixels);
            checkForGlErrors("texImage2D");
            glGenerateMipmap(GL_TEXTURE_2D);
            checkForGlErrors("asset load");
            img->texture = texture;

            SDL_FreeSurface(surface);
        });
    });

    return img;
}

template <> void AssetLoader<ImageData>::unloadAsset(ImageData *img) {
    GLuint texture = img->texture;
    TaskManager::instance->pushRender([texture](RenderContext &) {
        glDeleteTextures(1, &texture);
    });
    delete img;
}

template <> bool AssetLoader<ImageData>::assetIsReady(ImageData *img) {
    return img->texture > 0;
}

}
