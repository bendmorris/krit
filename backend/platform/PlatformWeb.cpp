#include "krit/platform/Platform.h"

namespace krit {

struct PlatformWeb : public Platform {
    PlatformWeb() {}

    std::string dataDir() override { return "/data"; }
    std::string configDir() override { return "/config"; }

    const char *name() override { return "web"; }
};

std::unique_ptr<Platform> platform() {
    return std::unique_ptr<Platform>(new PlatformWeb());
}

}
