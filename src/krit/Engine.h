#ifndef KRIT_ENGINE
#define KRIT_ENGINE

#include "krit/Camera.h"
#include "krit/Sprite.h"
#include "krit/asset/AssetCache.h"
#include "krit/input/InputContext.h"
#include "krit/utils/Color.h"
#include "krit/utils/Signal.h"
#include <list>
#include <memory>
#include <vector>

namespace krit {

struct TimedEvent {
    float delay;
    float interval;
    CustomSignal signal;
    void *userData;

    TimedEvent(float delay, float interval, CustomSignal signal, void *userData)
        : delay(delay), interval(interval), signal(signal), userData(userData) {
    }
};

struct RenderContext;
struct UpdateContext;
struct ImageData;
struct TextureAtlas;
struct Font;
struct BitmapFont;
struct SkeletonBinaryData;

struct SpriteTree {
    std::unique_ptr<Sprite> root;
    Camera *camera;

    SpriteTree(Sprite *root, Camera *camera) : root(root), camera(camera) {}
};

struct Engine {
    bool paused = false;
    bool fixedFrameRate = false;
    bool finished = false;
    float speed = 1;

    UpdateSignal onBegin = nullptr;
    UpdateSignal onUpdate = nullptr;
    UpdateSignal postUpdate = nullptr;
    RenderSignal onRender = nullptr;
    RenderSignal postRender = nullptr;
    std::list<TimedEvent> events;

    InputContext input;
    std::vector<AssetCache *> assetCaches;

    Color bgColor = Color::black();

    std::vector<SpriteTree> trees;
    void *userData = nullptr;

    Camera camera;
    Camera uiCamera;

    Engine() { assetCaches.emplace_back(new AssetCache()); }

    void update(UpdateContext &ctx);
    void fixedUpdate(UpdateContext &ctx);
    void render(RenderContext &ctx);

    void setTimeout(CustomSignal s, float delay = 0, void *userData = nullptr);
    void addTree(Sprite *root, Camera *camera = nullptr);
    Sprite *getRoot(int index) { return trees[index].root.get(); }
    void setRoot(int index, Sprite *root);
    void quit() { finished = true; }

    void pushAssetCache() { assetCaches.emplace_back(new AssetCache()); }
    void popAssetCache() {
        AssetCache *cache = assetCaches.back();
        assetCaches.pop_back();
        delete cache;
    }

    template <typename T> T *data() { return static_cast<T *>(this->userData); }

#define DECLARE_ASSET_GETTER(N, T)                                             \
    template <typename Arg> std::shared_ptr<T> get##N(const Arg &arg) {        \
        return assetCaches.back()->get<T>(arg);                                \
    }
    DECLARE_ASSET_GETTER(Image, ImageData)
    DECLARE_ASSET_GETTER(Atlas, TextureAtlas)
    DECLARE_ASSET_GETTER(Font, Font)
    DECLARE_ASSET_GETTER(BitmapFont, BitmapFont)
    DECLARE_ASSET_GETTER(Spine, SkeletonBinaryData)
};

}

#endif
