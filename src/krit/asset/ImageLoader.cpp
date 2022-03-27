#include "krit/App.h"
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
#include <string>

namespace krit {

struct RenderContext;
struct UpdateContext;

static char *yamlStr(yaml_node_t *node) {
    return static_cast<char *>(static_cast<void *>(node->data.scalar.value));
}

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

static std::unordered_map<std::string, ImageInfo> imageManifest;

void ImageLoader::parseManifest() {
    if (!IoRead::exists("assets/images.yaml")) {
        panic("couldn't find image manifest");
    }

    int len;
    char *manifest = IoRead::read("assets/images.yaml", &len);
    yaml_parser_t parser;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_string(&parser, (unsigned char *)manifest, len);

    yaml_document_t doc;
    if (!yaml_parser_load(&parser, &doc)) {
        panic("failed to parse asset manifest");
    }

    yaml_node_t *root = yaml_document_get_root_node(&doc);
    if (root->type != YAML_SEQUENCE_NODE) {
        panic("asset manifest didn't contain a top-level list");
    }
    for (yaml_node_item_t *it = root->data.sequence.items.start;
         it < root->data.sequence.items.top; ++it) {
        yaml_node_t *node = yaml_document_get_node(&doc, *it);
        if (node->type != YAML_MAPPING_NODE) {
            panic("asset manifest expected list of maps");
        }
        std::string path;
        int width = 0, height = 0;
        std::vector<ImageSize> sizes;
        for (yaml_node_pair_t *it = node->data.mapping.pairs.start;
             it < node->data.mapping.pairs.top; ++it) {
            yaml_node_t *keyNode = yaml_document_get_node(&doc, it->key);
            char *keyName = yamlStr(keyNode);
            yaml_node_t *value = yaml_document_get_node(&doc, it->value);
            if (!strcmp("path", keyName)) {
                path = yamlStr(value);
            } else if (!strcmp("width", keyName)) {
                width = atoi(yamlStr(value));
            } else if (!strcmp("height", keyName)) {
                height = atoi(yamlStr(value));
            } else if (!strcmp("sizes", keyName)) {
                for (yaml_node_pair_t *it = value->data.mapping.pairs.start;
                     it < value->data.mapping.pairs.top; ++it) {
                    yaml_node_t *keyNode =
                        yaml_document_get_node(&doc, it->key);
                    yaml_node_t *value =
                        yaml_document_get_node(&doc, it->value);
                    sizes.emplace_back(atoi(yamlStr(keyNode)), yamlStr(value));
                }
            } else {
                panic("unrecognized asset manifest key: %s", keyName);
            }
        }
        imageManifest.emplace(
            std::piecewise_construct, std::forward_as_tuple(path),
            std::forward_as_tuple(width, height, std::move(sizes)));
    }

    yaml_document_delete(&doc);
    yaml_parser_delete(&parser);

    IoRead::free(manifest);
}

template <>
std::shared_ptr<ImageData>
AssetLoader<ImageData>::loadAsset(const std::string &path) {
    // resolve path based on manifest alternates
    int width = 0, height = 0;
    float scale = 1.0;
    std::string pathToLoad;
    auto mfst = imageManifest.find(path);
    if (mfst == imageManifest.end()) {
        pathToLoad = path;
    } else {
        auto &info = mfst->second;
        int best, bestResolution;
        if (info.dimensions.x > 0 && info.dimensions.y > 0) {
            best = -1;
            bestResolution = 2160;
        } else {
            best = 0;
            bestResolution = info.sizes[0].resolution;
        }
        for (size_t i = 0; i < info.sizes.size(); ++i) {
            auto &size = info.sizes[i];
            if (size.resolution >= App::ctx.window->height() &&
                size.resolution < bestResolution) {
                best = i;
                bestResolution = size.resolution;
            }
        }
        if (best < 0) {
            pathToLoad = path;
            width = info.dimensions.x;
            height = info.dimensions.y;
        } else {
            pathToLoad = info.sizes[best].path;
            scale = static_cast<float>(bestResolution) / 2160.0;
            auto resolvedInfo = imageManifest.find(pathToLoad);
            if (resolvedInfo == imageManifest.end()) {
                panic("resolved image %s missing from manifest", pathToLoad.c_str());
            } else {
                width = resolvedInfo->second.dimensions.x / scale;
                height = resolvedInfo->second.dimensions.y / scale;
            }
        }
    }

    std::shared_ptr<ImageData> img = std::make_shared<ImageData>();
    img->dimensions.setTo(width, height);
    img->scale = scale;

#ifdef __EMSCRIPTEN__
    TaskManager::instance->push([info, img](UpdateContext &) {
        SDL_Surface *surface = IMG_Load(info.path.c_str());
        if (!surface) {
            panic("IMG_Load(%s) is null: %s\n", info.path.c_str(),
                  IMG_GetError());
        }
#else
    int len;
    char *s = IoRead::read(pathToLoad, &len);

    TaskManager::instance->push([=](UpdateContext &) {
        SDL_RWops *rw = SDL_RWFromConstMem(s, len);
        SDL_Surface *surface = IMG_LoadTyped_RW(rw, 0, "PNG");
        img->dimensions.setTo(surface->w / scale, surface->h / scale);
        SDL_RWclose(rw);
        IoRead::free(s);
        if (!surface) {
            panic("IMG_Load(%s) is null: %s\n", pathToLoad.c_str(),
                  IMG_GetError());
        }
#endif
        unsigned int mode =
            surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

        TaskManager::instance->pushRender([=](RenderContext &render) {
            // upload texture
            GLuint texture;
            glActiveTexture(GL_TEXTURE0);
            checkForGlErrors("active texture");
            glGenTextures(1, &texture);
            checkForGlErrors("gen textures");
            glBindTexture(GL_TEXTURE_2D, texture);
            checkForGlErrors("bind texture");
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
    return img->dimensions.x * img->dimensions.y;
}

template <> AssetType AssetLoader<ImageData>::type() { return ImageAsset; }

}
