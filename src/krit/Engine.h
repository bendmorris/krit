#ifndef KRIT_ENGINE
#define KRIT_ENGINE

#include "krit/Camera.h"
#include "krit/Sprite.h"
#include "krit/Window.h"
#include "krit/asset/AssetCache.h"
#include "krit/asset/Font.h"
#include "krit/input/InputContext.h"
#include "krit/render/Renderer.h"
#include "krit/sound/AudioBackend.h"
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
struct SkeletonBinaryData;
struct SoundData;
struct MusicData;

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
    float elapsed = 0;

    UpdateSignal onBegin = nullptr;
    UpdateSignal onEnd = nullptr;
    UpdateSignal onUpdate = nullptr;
    UpdateSignal postUpdate = nullptr;
    RenderSignal onRender = nullptr;
    RenderSignal postRender = nullptr;
    std::list<TimedEvent> events;
    std::vector<std::unique_ptr<AssetCache>> assetCaches;

    Window window;
    Renderer renderer;
    FontManager fonts;
    AudioBackend audio;
    InputContext input;

    Color bgColor = Color::black();

    std::vector<SpriteTree> trees;
    void *userData = nullptr;

    Camera camera;
    Camera uiCamera;

    Engine(KritOptions &options);

    void update(UpdateContext &ctx);
    void fixedUpdate(UpdateContext &ctx);
    void render(RenderContext &ctx);

    void setTimeout(CustomSignal s, float delay = 0, void *userData = nullptr);
    void addTree(Sprite *root, Camera *camera = nullptr);
    Sprite *getRoot(int index) { return trees[index].root.get(); }
    void setRoot(int index, Sprite *root);
    void quit() { finished = true; }

    void pushAssetCache() {
        assetCaches.emplace_back(std::make_unique<AssetCache>());
    }
    void popAssetCache() { assetCaches.pop_back(); }

    template <typename T> T *data() { return static_cast<T *>(this->userData); }

#define DECLARE_ASSET_GETTER(N, T)                                             \
    template <typename Arg>                                                    \
    std::shared_ptr<T> get##N(const Arg &arg, bool cache = true) {             \
        return cache ? assetCaches.back()->get<T>(arg)                         \
                     : AssetCache::load<T>(arg);                               \
    }
    DECLARE_ASSET_GETTER(Image, ImageData)
    DECLARE_ASSET_GETTER(Atlas, TextureAtlas)
    DECLARE_ASSET_GETTER(Font, Font)
    DECLARE_ASSET_GETTER(Spine, SkeletonBinaryData)
    DECLARE_ASSET_GETTER(Sound, SoundData)
    DECLARE_ASSET_GETTER(Music, MusicData)
    DECLARE_ASSET_GETTER(Text, std::string_view)
#undef DECLARE_ASSET_GETTER
};

}

#endif
