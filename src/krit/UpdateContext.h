#ifndef KRIT_UPDATE_CONTEXT
#define KRIT_UPDATE_CONTEXT

namespace krit {

class App;
class Camera;
class Engine;
class AssetContext;
class ControlBindings;
class InputContext;

struct UpdateContext {
    double elapsed;
    unsigned int frameCount;
    unsigned int frameId;
    App *app = nullptr;
    Engine *engine = nullptr;
    IntDimensions *window = nullptr;
    Camera *camera = nullptr;
    AssetContext *asset = nullptr;
    ControlBindings *controls = nullptr;
    InputContext *input = nullptr;
    void *userData = nullptr;

    template <typename T> T *data() { return static_cast<T*>(this->userData); }
};

}

#endif
