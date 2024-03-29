#ifndef KRIT_L12N_TEXTMAP
#define KRIT_L12N_TEXTMAP

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace krit {

struct TextMap {
    static TextMap *create() { return new TextMap(); }

    std::string locale;

    void registerLocale(const std::string &key, const std::string &asset);
    void setLocale(const std::string &key);
    void setCurrentLocaleAsDefault();
    std::string_view getString(const std::string &key);

private:
    std::unordered_map<std::string, std::string> locales;
    std::unordered_map<std::string_view, std::string_view> strings;
    std::unordered_map<std::string_view, std::string_view> defaultStrings;
    std::shared_ptr<std::string> loaded = nullptr;
    std::shared_ptr<std::string> loadedDefault = nullptr;
};

}

#endif
