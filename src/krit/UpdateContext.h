#ifndef KRIT_UPDATE_CONTEXT
#define KRIT_UPDATE_CONTEXT

#include "krit/math/Dimensions.h"

namespace krit {

struct App;
struct AudioBackend;
struct Camera;
struct Engine;
struct InputContext;
struct Window;

struct UpdateContext {
    float elapsed;
    unsigned int frameCount = 0;
    unsigned int frameId = 0;
    unsigned int tickId = 0;

    UpdateContext() {}
    UpdateContext(const UpdateContext &) {}
};

}

#endif
