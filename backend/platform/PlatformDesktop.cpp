#include "krit/platform/Platform.h"
#include "tinyfiledialogs.h"

namespace krit {

struct PlatformDesktop : public Platform {
    PlatformDesktop() {}

    std::string dataDir() override {
        static std::string path;
        if (path.empty()) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            // windows
            path = std::string(getenv("APPDATA"));
#elif __APPLE__
            // mac
            // TODO
#else
            // linux
            char *xdgConfig = getenv("XDG_DATA_HOME");
            if (xdgConfig) {
                path = std::string(xdgConfig);
            } else {
                path = std::string(getenv("HOME")) + "/.local/share";
            }
#endif
        }
        // TODO: mkdir
        return path;
    }
    std::string configDir() override {
        static std::string path;
        if (path.empty()) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            // windows
            path = std::string(getenv("PROGRAMDATA"));
#elif __APPLE__
            // mac
            // TODO
#else
            // linux
            char *xdgConfig = getenv("XDG_CONFIG_HOME");
            if (xdgConfig) {
                path = std::string(xdgConfig);
            } else {
                path = std::string(getenv("HOME")) + "/.config";
            }
#endif
        }
        // TODO: mkdir
        return path;
    }

    const char *name() override {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        return "windows";
#elif __APPLE__
        return "apple";
#else
        return "linux";
#endif
    }

    std::optional<std::string>
    saveFileDialog(const std::string &title,
                   std::vector<std::string> filters) override {
        const char *desc = filters.empty() ? "" : filters[0].c_str();
        std::vector<const char *> filterPtrs;
        for (size_t i = 1; i < filters.size(); ++i) {
            filterPtrs.push_back(filters[i].c_str());
        }
        const char *result = tinyfd_saveFileDialog(
            title.c_str(), "", filterPtrs.size(), &filterPtrs[0], desc);
        return result ? std::optional<std::string>(result) : std::nullopt;
    }

    std::optional<std::string>
    openFileDialog(const std::string &title,
                   std::vector<std::string> filters) override {
        const char *desc = filters.empty() ? "" : filters[0].c_str();
        std::vector<const char *> filterPtrs;
        for (size_t i = 1; i < filters.size(); ++i) {
            filterPtrs.push_back(filters[i].c_str());
        }
        const char *result = tinyfd_openFileDialog(
            title.c_str(), "", filterPtrs.size(), &filterPtrs[0], desc, 0);
        return result ? std::optional<std::string>(result) : std::nullopt;
    }
};

std::unique_ptr<Platform> platform() {
    return std::unique_ptr<Platform>(new PlatformDesktop());
}

}
