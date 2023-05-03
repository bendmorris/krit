#include "krit/platform/Platform.h"

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
};

std::unique_ptr<Platform> platform() {
    return std::unique_ptr<Platform>(new PlatformDesktop());
}

}
