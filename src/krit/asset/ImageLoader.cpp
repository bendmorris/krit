#include "krit/Engine.h"
#include "krit/TaskManager.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/math/Dimensions.h"
#include "krit/render/Gl.h"
#include "krit/render/ImageData.h"
#include "krit/utils/Log.h"
#include "krit/utils/Panic.h"
#include "yaml.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include <cstdint>
#include <filesystem>
#include <jpeglib.h>
#include <png.h>
#include <string>

namespace krit {

struct RenderContext;
struct UpdateContext;

struct PngData {
    const char *data;
    size_t length;
};

static void png_read_custom(png_structp png_ptr, png_bytep outBytes,
                            png_size_t byteCountToRead) {
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    PngData *io = static_cast<PngData *>(io_ptr);
    if (byteCountToRead > io->length) {
        byteCountToRead = io->length;
    }
    if (io->length > 0) {
        memcpy(outBytes, io->data, byteCountToRead);
        io->length -= byteCountToRead;
        io->data += byteCountToRead;
    }
}

struct ImageResolutionInfo {
    std::filesystem::path extension;
    int resolution;
};

// FIXME: this should be dynamic
static std::unordered_map<std::string, std::vector<ImageResolutionInfo>>
    resolutions{
        {".png", {{".720.png", 720}, {".1080.png", 1080}, {".png", 2160}}},
        {".jpg", {{".720.jpg", 720}, {".1080.jpg", 1080}, {".jpg", 2160}}},
    };

struct ImageSize {
    int resolution;
    std::string path;

    ImageSize(int resolution, const char *s)
        : resolution(resolution), path(s) {}
};

struct ImageInfo {
    IntDimensions dimensions;
    std::vector<ImageSize> sizes;

    ImageInfo(int w, int h, std::vector<ImageSize> &&sizes)
        : dimensions(w, h), sizes(sizes) {}
};

template <>
std::shared_ptr<ImageData>
AssetLoader<ImageData>::loadAsset(const std::string &key) {
    std::shared_ptr<ImageData> img = std::make_shared<ImageData>();

    std::filesystem::path path = key;
    std::string extension = path.extension().string();

    int windowHeight = engine->window.y();

    ImageResolutionInfo *found = nullptr;
    std::filesystem::path pathToLoad;
    std::filesystem::path foundArchive;
    for (auto &res : resolutions[extension]) {
        if ((!found || res.resolution >= windowHeight)) {
            pathToLoad = path;
            pathToLoad.replace_extension();
            pathToLoad += res.extension;
            auto foundAsset = engine->io->find(pathToLoad);
            if (foundAsset) {
                found = &res;
                foundArchive = std::move(*foundAsset);
                if (res.resolution >= windowHeight) {
                    break;
                }
            }
        }
    }
    if (!found) {
        LOG_ERROR("Couldn't find a valid resolution for image: %s",
                  key.c_str());
        return img;
    }

    pathToLoad = path;
    pathToLoad.replace_extension();
    pathToLoad += found->extension;

    float scale = found->resolution / 2160.0;

    std::string s = engine->io->readFile(pathToLoad.c_str());

    const char *imgType;
    if (extension == ".png") {
        imgType = "PNG";
        // use libpng to get the dimensions
        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                     nullptr, nullptr, nullptr);
        if (!png_ptr) {
            LOG_ERROR("Failed to initialize PNG read struct: %s", key.c_str());
            return img;
        }
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            LOG_ERROR("Failed to initialize PNG info struct: %s", key.c_str());
            return img;
        }
        PngData png{.data = s.c_str(), .length = s.size()};
        png_set_read_fn(png_ptr, &png, png_read_custom);
        png_read_info(png_ptr, info_ptr);
        png_uint_32 width, height;
        png_uint_32 ret =
            png_get_IHDR(png_ptr, info_ptr, &width, &height, nullptr, nullptr,
                         nullptr, nullptr, nullptr);
        if (ret != 1) {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            LOG_ERROR("Failed to read PNG header: %s", key.c_str());
            return img;
        }
        // printf("%s: %u x %u\n", pathToLoad.c_str(), width, height);
        img->dimensions.setTo(width / scale, height / scale);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    } else if (extension == ".jpg") {
        imgType = "JPEG";
        // use libjpeg to get the dimensions
        static jpeg_error_mgr jerr;
        jpeg_std_error(&jerr);
        struct jpeg_decompress_struct cinfo;
        cinfo.err = &jerr;
        jpeg_create_decompress(&cinfo);
        jpeg_mem_src(&cinfo, (const unsigned char *)(void *)s.c_str(),
                     s.size());
        int ret = jpeg_read_header(&cinfo, TRUE);
        if (ret != JPEG_HEADER_OK) {
            jpeg_destroy_decompress(&cinfo);
            LOG_ERROR("Failed to read JPEG header: %s", key.c_str());
            return img;
        }
        img->dimensions.setTo(cinfo.image_width / scale,
                              cinfo.image_height / scale);
        jpeg_destroy_decompress(&cinfo);
    } else {
        LOG_ERROR("Unrecognized image extension for image: %s", key.c_str());
        return img;
    }

    img->scale = scale;

#ifdef __EMSCRIPTEN__
    // emscripten loads images by path
    (void)imgType;
    TaskManager::instance->push(
        [=, pathToLoad = std::move(pathToLoad)]() mutable {
            LOG_DEBUG("load image %s", pathToLoad.c_str());
            auto fullPathToLoad = foundArchive / pathToLoad;
            SDL_Surface *surface = IMG_Load(fullPathToLoad.c_str());
            if (!surface) {
                panic("IMG_Load(%s) is null: %s\n", fullPathToLoad.c_str(),
                      IMG_GetError());
            }
#else
    TaskManager::instance->push([=, pathToLoad = std::move(pathToLoad),
                                 s = std::move(s)]() mutable {
        LOG_DEBUG("load image %s", pathToLoad.c_str());
        SDL_RWops *rw = SDL_RWFromConstMem(s.c_str(), s.size());
        SDL_Surface *surface = IMG_LoadTyped_RW(rw, 0, imgType);
        if (!surface) {
            panic("IMG_Load(%s) is null: %s\n", pathToLoad.c_str(),
                  IMG_GetError());
        }
        img->dimensions.setTo(surface->w / img->scale, surface->h / img->scale);
        SDL_RWclose(rw);
#endif
            bool hasAlpha = surface->format->Amask;
            // uint32_t desiredFormat =
            //     hasAlpha ? SDL_PIXELFORMAT_ABGR8888 : SDL_PIXELFORMAT_BGR888;
            // if (surface->format->format != desiredFormat) {
            //     // convert the format
            //     SDL_Surface *oldSurface = surface;
            //     surface = SDL_ConvertSurfaceFormat(oldSurface, desiredFormat,
            //     0); SDL_FreeSurface(oldSurface);
            // }
            unsigned int mode;
            if (hasAlpha) {
                if (surface->format->Rmask == 0x000000ff) {
                    mode = GL_RGBA;
                } else {
                    mode = GL_BGRA;
                }
            } else if (surface->format->BytesPerPixel == 3) {
                if (surface->format->Rmask == 0x000000ff) {
                    mode = GL_RGB;
                } else {
                    mode = GL_BGR;
                }
            } else if (surface->format->BytesPerPixel == 1) {
                mode = GL_RED;
            } else {
                panic("IMAGE_Load(%s): BytesPerPixel=%u", pathToLoad.c_str(),
                      surface->format->BytesPerPixel);
            }

            TaskManager::instance->pushRender([=]() {
                LOG_DEBUG("callback: load image %s",
                          pathToLoad.c_str(), surface->format->format,
                          (int)surface->format->BitsPerPixel, surface->pitch);
                // upload texture
                GLuint texture;
                glActiveTexture(GL_TEXTURE0);
                checkForGlErrors("active texture");
                glGenTextures(1, &texture);
                if (!texture) {
                    LOG_ERROR("failed to generate texture for image %s",
                              pathToLoad.c_str());
                }
                checkForGlErrors("gen textures");
                glBindTexture(GL_TEXTURE_2D, texture);
                checkForGlErrors("bind texture");
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0,
                             mode, GL_UNSIGNED_BYTE, surface->pixels);
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
    return (img->dimensions.x() * img->scale) *
           (img->dimensions.y() * img->scale) * 4;
}

template <> AssetType AssetLoader<ImageData>::type() { return ImageAsset; }
}
