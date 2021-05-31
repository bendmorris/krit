#include "krit/asset/AssetCache.h"
#include "krit/App.h"
#include "krit/asset/BitmapFont.h"
#include "krit/utils/Panic.h"
#include "krit/Assets.h"
#include "expat.h"
#include <cstring>
#include <utility>

namespace krit {

void startElement(void *userData, const char *name, const char **attrs) {
    BitmapFont *font = static_cast<BitmapFont*>(userData);
    if (!strcmp(name, "kernings")) {
        const char **attr = attrs;
        while (*attr) {
            if (*(attr + 1)) {
                const char *key = *attr;
                const char *value = *(attr + 1);
                if (!strcmp(key, "count")) {
                    font->kerningTable.reserve(atoi(value));
                }
            }
            attr = attr + 2;
        }
    } else if (!strcmp(name, "kerning")) {
        int first, second, amount;
        const char **attr = attrs;
        while (*attr) {
            if (*(attr + 1)) {
                const char *key = *attr;
                const char *value = *(attr + 1);
                if (!strcmp(key, "first")) {
                    first = atoi(value);
                } else if (!strcmp(key, "second")) {
                    second = atoi(value);
                } else if (!strcmp(key, "amount")) {
                    amount = atoi(value);
                }
            }
            attr = attr + 2;
        }
        int64_t key = (static_cast<int64_t>(first) << 32) | second;
        font->kerningTable.insert(std::make_pair(key, amount));
    } else if (!strcmp(name, "info")) {
        const char **attr = attrs;
        while (*attr) {
            if (*(attr + 1)) {
                const char *key = *attr;
                const char *value = *(attr + 1);
                if (!strcmp(key, "size")) {
                    font->size = atoi(value);
                }
            }
            attr = attr + 2;
        }
    } else if (!strcmp(name, "common")) {
        const char **attr = attrs;
        while (*attr) {
            if (*(attr + 1)) {
                const char *key = *attr;
                const char *value = *(attr + 1);
                if (!strcmp(key, "lineHeight")) {
                    font->lineHeight = atoi(value);
                }
            }
            attr = attr + 2;
        }
    } else if (!strcmp(name, "page")) {
        const char **attr = attrs;
        while (*attr) {
            if (*(attr + 1)) {
                const char *key = *attr;
                const char *value = *(attr + 1);
                if (!strcmp(key, "file")) {
                    std::shared_ptr<ImageData> page = App::ctx.engine->getImage(value);
                    font->pages.emplace_back(page);
                }
            }
            attr = attr + 2;
        }
    } else if (!strcmp(name, "char")) {
        BitmapGlyphData glyphData;
        const char **attr = attrs;
        while (*attr) {
            if (*(attr + 1)) {
                const char *key = *attr;
                const char *value = *(attr + 1);
                if (!strcmp(key, "id")) {
                    glyphData.id = atoi(value);
                } else if (!strcmp(key, "x")) {
                    glyphData.rect.x = atoi(value);
                } else if (!strcmp(key, "y")) {
                    glyphData.rect.y = atoi(value);
                } else if (!strcmp(key, "width")) {
                    glyphData.rect.width = atoi(value);
                } else if (!strcmp(key, "height")) {
                    glyphData.rect.height = atoi(value);
                } else if (!strcmp(key, "xoffset")) {
                    glyphData.offset.x = atoi(value);
                } else if (!strcmp(key, "yoffset")) {
                    glyphData.offset.y = atoi(value);
                } else if (!strcmp(key, "xadvance")) {
                    glyphData.xAdvance = atoi(value);
                } else if (!strcmp(key, "page")) {
                    glyphData.page = atoi(value);
                }
            }
            attr = attr + 2;
        }
        if (glyphData.id) {
            font->glyphData[glyphData.id] = glyphData;
        }
    }
}

BitmapFont::BitmapFont(const char *path): BitmapFontBase() {
    // font
    //   info: size
    //   common: lineheight
    //   pages
    //     page: file, id
    //   chars (+count)
    //     char: x, y, width, height, xoffset, yoffset, xadvance, page
    XML_Parser parser = XML_ParserCreate(nullptr);
    XML_SetUserData(parser, this);
    XML_SetElementHandler(parser, startElement, nullptr);
    char buf[1024];
    FILE *fp = fopen(path, "rb");
    do {
        int readLength = fread(buf, 1, 1024, fp);
        if (!readLength) {
            break;
        }
        if (XML_Parse(parser, buf, readLength, readLength < 1024 ? 1 : 0) == XML_STATUS_ERROR) {
            panic("%s: failed to parse bitmap font XML!", path);
        }
    } while (true);
    XML_ParserFree(parser);
}

template<> BitmapFont *AssetLoader<BitmapFont>::loadAsset(const AssetInfo &info) {
    BitmapFont *font = new BitmapFont(info.path.c_str());
    return font;
}

template<> void AssetLoader<BitmapFont>::unloadAsset(BitmapFont *font) {
    delete font;
}

}
