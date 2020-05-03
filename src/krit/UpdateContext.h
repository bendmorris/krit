#ifndef KRIT_UPDATE_CONTEXT
#define KRIT_UPDATE_CONTEXT

#include "krit/math/Dimensions.h"

namespace krit {

struct App;
struct Camera;
struct Engine;
struct AssetContext;
struct ControlBindings;
struct InputContext;

struct UpdateContext {
    double elapsed;
    unsigned int frameCount;
    unsigned int frameId;
    krit::App *app = nullptr;
    krit::Engine *engine = nullptr;
    krit::IntDimensions *window = nullptr;
    krit::Camera *camera = nullptr;
    krit::AssetContext *asset = nullptr;
    krit::ControlBindings *controls = nullptr;
    krit::InputContext *input = nullptr;
    void *userData = nullptr;

    UpdateContext() {}
    UpdateContext(const UpdateContext &) {}

    template <typename T> T *data() { return static_cast<T*>(this->userData); }
};

}

#endif
