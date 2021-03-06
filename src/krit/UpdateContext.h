#ifndef KRIT_UPDATE_CONTEXT
#define KRIT_UPDATE_CONTEXT

#include "krit/math/Dimensions.h"

namespace krit {

struct App;
struct Camera;
struct Engine;
struct InputContext;

struct UpdateContext {
    float elapsed;
    unsigned int frameCount;
    unsigned int frameId;
    krit::App *app = nullptr;
    krit::Engine *engine = nullptr;
    krit::IntDimensions *window = nullptr;
    krit::Camera *camera = nullptr;
    void *userData = nullptr;

    UpdateContext() {}
    UpdateContext(const UpdateContext &) {}

    template <typename T> T *data() { return static_cast<T *>(this->userData); }
};

}

#endif
