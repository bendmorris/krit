#ifndef KRIT_ENGINE
#define KRIT_ENGINE

#include "krit/Camera.h"
#include "krit/Window.h"
#include "krit/asset/AssetCache.h"
#include "krit/asset/Font.h"
#include "krit/input/InputContext.h"
#include "krit/render/Renderer.h"
#if KRIT_ENABLE_SCRIPT
#include "krit/script/ScriptEngine.h"
#endif
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
struct ParticleEffect;
struct SkeletonBinaryData;
struct SoundData;
struct SpineData;
struct MusicData;

struct Engine {
    bool paused = false;
    bool fixedFrameRate = false;
    bool finished = false;
    float speed = 1;
    float elapsed = 0;

    UpdateSignal onBegin = nullptr;
    UpdateSignal onEnd = nullptr;
    UpdateSignal onUpdate = nullptr;
    UpdateSignal onFixedUpdate = nullptr;
    UpdateSignal postUpdate = nullptr;
    RenderSignal onRender = nullptr;
    RenderSignal postRender = nullptr;
    std::list<TimedEvent> events;

    Window window;
    Renderer renderer;
    FontManager fonts;
    AudioBackend audio;
    InputContext input;
    AssetCache assets;
#if KRIT_ENABLE_SCRIPT
    ScriptEngine script;
#endif
    std::unordered_map<std::string, std::vector<std::pair<int, SDL_Cursor *>>>
        cursors;

    Color bgColor = Color::black();

    void *userData = nullptr;

    std::vector<Camera> cameras;

    std::string cursor;

    Engine(KritOptions &options);

    void update(UpdateContext &ctx);
    void fixedUpdate(UpdateContext &ctx);
    void render(RenderContext &ctx);
    void flip(RenderContext &ctx);

    void setTimeout(CustomSignal s, float delay = 0, void *userData = nullptr);
    void quit() { finished = true; }

    void addCursor(const std::string &cursorPath, const std::string &cursor,
                   int resolution);
    void setCursor(const std::string &cursor);

    Camera &addCamera() {
        cameras.emplace_back();
        return cameras.back();
    }

    template <typename T> T *data() { return static_cast<T *>(this->userData); }

#define DECLARE_ASSET_GETTER(N, T)                                             \
    template <typename Arg> std::shared_ptr<T> get##N(const Arg &arg) {        \
        return assets.get<T>(arg);                                             \
    }
    DECLARE_ASSET_GETTER(Image, ImageData)
    DECLARE_ASSET_GETTER(Atlas, TextureAtlas)
    DECLARE_ASSET_GETTER(Font, Font)
    DECLARE_ASSET_GETTER(Spine, SpineData)
    DECLARE_ASSET_GETTER(Sound, SoundData)
    DECLARE_ASSET_GETTER(Music, MusicData)
    DECLARE_ASSET_GETTER(Text, std::string_view)
    DECLARE_ASSET_GETTER(Particle, ParticleEffect)
#undef DECLARE_ASSET_GETTER

private:
    void chooseCursor();
    SDL_Cursor *_cursor = nullptr;
};

}

#endif
