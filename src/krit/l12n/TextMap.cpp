#include "krit/l12n/TextMap.h"
#include "krit/asset/AssetLoader.h"
#include "krit/utils/Log.h"
#include <cstring>
#include <string_view>

namespace krit {

void TextMap::registerLocale(const std::string &key, const std::string &path) {
    locales[key] = path;
}

void TextMap::setLocale(const std::string &key) {
    if (locale != key) {
        strings.clear();
        locale = key;
        loaded = AssetLoader<std::string>::loadAsset(locales[key]);
        const char *current = loaded->data();
        size_t remaining = loaded->length();
        const char *nextNewline, *nextTab;
        int line = 0;
        while ((nextNewline = (const char *)memchr(current, '\n', remaining))) {
            ++line;
            size_t lineLength = nextNewline - current;
            nextTab = (const char *)memchr(current, '\t', lineLength);
            if (!nextTab) {
                Log::error("no tab in line %i: %.*s\n", line, (int)lineLength - 1, current);
                current = nextNewline + 1;
                remaining -= lineLength + 1;
                continue;
            }
            strings[std::string_view(current, nextTab - current)] =
                std::string_view(nextTab + 1, nextNewline - nextTab - 1);
            // printf("%.*s: %.*s\n", (int)(nextTab - current), current,
            // (int)(nextNewline - nextTab - 1), nextTab + 1);
            current = nextNewline + 1;
            remaining -= lineLength + 1;
        }
    }
}

std::string_view TextMap::getString(const std::string &key) {
    std::string_view _key(key.data(), key.length());
    auto found = strings.find(_key);
    if (found == strings.end()) {
        return defaultStrings[_key];
    } else {
        return found->second;
    }
}

void TextMap::setCurrentLocaleAsDefault() {
    loadedDefault = loaded;
    defaultStrings = strings;
}

}
