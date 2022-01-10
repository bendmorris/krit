#include "krit/App.h"
#include "krit/TaskManager.h"
#include "krit/asset/AssetInfo.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/math/Dimensions.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include "krit/utils/Log.h"
#include "krit/utils/Panic.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include <cstdint>
#include <string>

namespace krit {

struct RenderContext;
struct UpdateContext;

template <>
std::shared_ptr<ImageData>
AssetLoader<ImageData>::loadAsset(const AssetInfo &info) {
    std::shared_ptr<ImageData> img = std::make_shared<ImageData>();
    img->dimensions.setTo(info.properties.img.dimensions);
    img->scale = info.properties.img.scale;
#ifdef __EMSCRIPTEN__
    TaskManager::instance->push([info, img](UpdateContext &) {
        SDL_Surface *surface = IMG_Load(info.path.c_str());
        if (!surface) {
            panic("IMG_Load(%s) is null: %s\n", info.path.c_str(),
                  IMG_GetError());
        }
#else
    int len;
    char *s = IoRead::read(info.path, &len);

    TaskManager::instance->push([info, img, s, len](UpdateContext &) {
        SDL_RWops *rw = SDL_RWFromConstMem(s, len);
        SDL_Surface *surface = IMG_LoadTyped_RW(rw, 0, "PNG");
        SDL_RWclose(rw);
        IoRead::free(s);
        if (!surface) {
            panic("IMG_Load(%s) is null: %s\n", info.path.c_str(),
                  IMG_GetError());
        }
#endif
        unsigned int mode =
            surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

        TaskManager::instance->pushRender(
            [info, img, surface, mode](RenderContext &render) {
                // upload texture
                GLuint texture;
                glActiveTexture(GL_TEXTURE0);
                checkForGlErrors("active texture");
                glGenTextures(1, &texture);
                checkForGlErrors("gen textures");
                glBindTexture(GL_TEXTURE_2D, texture);
                checkForGlErrors("bind texture");
                glTexImage2D(GL_TEXTURE_2D, 0, mode,
                             info.properties.img.realDimensions.x,
                             info.properties.img.realDimensions.y, 0, mode,
                             GL_UNSIGNED_BYTE, surface->pixels);
                checkForGlErrors("texImage2D");
                glGenerateMipmap(GL_TEXTURE_2D);
                checkForGlErrors("asset load");
                glBindTexture(GL_TEXTURE_2D, 0);
                img->texture = texture;
                SDL_FreeSurface(surface);
            });
    });

    return img;
}

template <> bool AssetLoader<ImageData>::assetIsReady(ImageData *img) {
    return img->texture > 0;
}

template <> size_t AssetLoader<ImageData>::cost(ImageData *img) {
    return img->dimensions.x * img->dimensions.y;
}

template <> AssetType AssetLoader<ImageData>::type() { return ImageAsset; }

}
