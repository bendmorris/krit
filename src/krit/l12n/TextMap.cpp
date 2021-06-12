#include "krit/l12n/TextMap.h"
#include "krit/asset/AssetLoader.h"
#include <cstring>
#include <string_view>

namespace krit {

void TextMap::registerLocale(const std::string &key, const AssetInfo &asset) {
    locales[key] = &asset;
}

void TextMap::setLocale(const std::string &key) {
    locale = key;
    if (loaded) {
        AssetLoader<std::string_view>::unloadAsset(loaded);
        loaded = nullptr;
    }
    loaded = AssetLoader<std::string_view>::loadAsset(*locales[key]);
    const char *current = loaded->data();
    size_t remaining = loaded->length();
    const char *nextNewline, *nextTab;
    while ((nextNewline = (const char*)memchr(current, '\n', remaining))) {
        size_t lineLength = nextNewline - current;
        nextTab = (const char*)memchr(current, '\t', lineLength);
        if (!nextTab) {
            continue;
        }
        strings[std::string_view(current, nextTab - current)] = std::string_view(nextTab + 1, nextNewline - nextTab - 1);
        // printf("%.*s: %.*s\n", (int)(nextTab - current), current, (int)(nextNewline - nextTab - 1), nextTab + 1);
        current = nextNewline + 1;
        remaining -= lineLength + 1;
    }
}

std::string TextMap::getString(const std::string &key) {
    auto s = strings[std::string_view(key.data(), key.length())];
    return std::string(s.data(), s.size());
}


}
