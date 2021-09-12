#ifndef KRIT_UPDATE_CONTEXT
#define KRIT_UPDATE_CONTEXT

#include "krit/math/Dimensions.h"

namespace krit {

struct App;
struct Camera;
struct Engine;
struct InputContext;
struct AudioBackend;

struct UpdateContext {
    float elapsed;
    unsigned int frameCount;
    unsigned int frameId;
    App *app = nullptr;
    Engine *engine = nullptr;
    IntDimensions *window = nullptr;
    Camera *camera = nullptr;
    AudioBackend *audio = nullptr;
    void *userData = nullptr;

    UpdateContext() {}
    UpdateContext(const UpdateContext &) {}

    template <typename T> T *data() { return static_cast<T *>(this->userData); }
};

}

#endif
