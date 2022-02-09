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
    App *app = nullptr;
    Engine *engine = nullptr;
    Camera *camera = nullptr;
    AudioBackend *audio = nullptr;
    Window *window = nullptr;
    void *userData = nullptr;

    UpdateContext() {}
    UpdateContext(const UpdateContext &) {}

    template <typename T> T *data() { return static_cast<T *>(this->userData); }
};

}

#endif
