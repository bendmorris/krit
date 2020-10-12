#ifndef KRIT_ENGINE
#define KRIT_ENGINE

#include "krit/asset/AssetCache.h"
#include "krit/input/ControlBindings.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Color.h"
#include "krit/utils/Signal.h"
#include "krit/Camera.h"
#include "krit/Sprite.h"
#include <list>

namespace krit {

struct TimedEvent {
    double delay;
    double interval;
    CustomSignal signal;
    void *userData;

    TimedEvent(double delay, double interval, CustomSignal signal, void *userData)
        : delay(delay), interval(interval), signal(signal), userData(userData) {}
};

struct AssetContext;
struct InputContext;
struct Sprite;
struct Editor;

struct SpriteTree {
    std::unique_ptr<Sprite> root;
    Camera *camera;

    SpriteTree(Sprite *root, Camera *camera): root(root), camera(camera) {}
};

struct Engine {
    bool paused = false;
    bool fixedFrameRate = false;
    bool finished = false;

    UpdateSignal onBegin = nullptr;
    UpdateSignal onUpdate = nullptr;
    UpdateSignal postUpdate = nullptr;
    RenderSignal onRender = nullptr;
    RenderSignal postRender = nullptr;
    std::list<TimedEvent> events;

    InputContext *input;
    AssetContext *asset;

    AssetCache assetCache;
    ControlBindings controls;

    Color bgColor = Color::black();

    std::vector<SpriteTree> trees;
    void *userData = nullptr;

    Camera camera;
    Camera uiCamera;

    Engine() {}

    void update(UpdateContext &ctx);
    void fixedUpdate(UpdateContext &ctx);
    void render(RenderContext &ctx);
    
    void setTimeout(CustomSignal s, double delay = 0, void *userData = nullptr);
    void addTree(Sprite *root, Camera *camera = nullptr);
    Sprite *getRoot(int index) { return trees[index].root.get(); }
    void setRoot(int index, Sprite *root);
    void reset() { controls.reset(); }
    void quit() { finished = true; }

    template <typename T> T *data() { return static_cast<T*>(this->userData); }
};

}

#endif
