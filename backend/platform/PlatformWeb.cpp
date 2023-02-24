#include "krit/platform/Platform.h"

namespace krit {

struct PlatformWeb : public Platform {
    PlatformWeb() {}

    void mainLoop() override {}
    const char *dataDir() override {
        return "/data";
    }
    const char *configDir() override {
        return "/config";
    }

    const char *name() override {
        return "web";
    }
};

std::unique_ptr<Platform> platform() {
    return std::unique_ptr<Platform>(new PlatformWeb());
}

}
