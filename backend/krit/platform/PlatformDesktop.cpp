#include "Platform.h"

namespace krit {

struct PlatformDesktop : public Platform {
    PlatformDesktop() {}

    void mainLoop() override {}
    const char *dataDir() override {
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
        return path.c_str();
    }
    const char *configDir() override {
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
        return path.c_str();
    }
};

std::unique_ptr<Platform> platform() {
    return std::unique_ptr<Platform>(new PlatformDesktop());
}

}
