#ifndef KRIT_PLATFORM
#define KRIT_PLATFORM

#include <memory>

namespace krit {

struct Platform {
    virtual ~Platform() = default;
    virtual void mainLoop() = 0;
    virtual const char *dataDir() = 0;
    virtual const char *configDir() = 0;
    virtual const char *name() = 0;
};

std::unique_ptr<Platform> platform();

}

#endif
