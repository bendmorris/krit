#ifndef KRIT_ENGINE
#define KRIT_ENGINE

#include "krit/Camera.h"
#include "krit/Options.h"
#include "krit/Window.h"
#include "krit/asset/AssetCache.h"
#include "krit/asset/Font.h"
#include "krit/input/InputContext.h"
#include "krit/io/Io.h"
#include "krit/math/Dimensions.h"
#include "krit/net/Net.h"
#include "krit/platform/Platform.h"
#include "krit/render/Renderer.h"
#include "krit/script/ScriptEngine.h"
#include "krit/sound/AudioBackend.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"
#include "krit/utils/Signal.h"
#include <chrono>
#include <string>

namespace krit {

struct TaskManager;
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

const int MAX_FRAMES = 5;
const int FPS = 60;

struct Engine;

extern Engine *engine;

struct Engine {
    enum class FramePhase { Inactive, Begin, Update, Render };

    struct TimedEvent {
        using SignalType = ReturnSignal<bool, void *>;
        float delay;
        float interval;
        SignalType signal;
        void *userData;

        TimedEvent(float delay, float interval, SignalType signal,
                   void *userData)
            : delay(delay), interval(interval), signal(signal),
              userData(userData) {}
    };

    bool isRenderPhase() { return phase == FramePhase::Render; }

private:
    struct EngineScope {
        EngineScope(Engine *);
        ~EngineScope();
    };

    EngineScope _scope;

public:
    // backends
    std::unique_ptr<Io> io;
    std::unique_ptr<Net> net;
    std::unique_ptr<Platform> platform;

    RenderContext ctx;
    Window window;
    Renderer renderer;
    FontManager fonts;
    AudioBackend audio;
    InputContext input;
    AssetCache assets;
    ScriptEngine script;
    std::unordered_map<std::string, std::vector<std::pair<int, SDL_Cursor *>>>
        cursors;

    FramePhase phase = FramePhase::Inactive;
    int fixedFramerate = 0;
    bool running = false;
    bool paused = false;
    bool fixedFrameRate = false;
    float speed = 1;
    double totalElapsed = 0;

    Signal onBegin;
    Signal onEnd;
    Signal onUpdate;
    Signal onFixedUpdate;
    Signal postUpdate;
    Signal onRender;
    Signal postRender;
    std::list<TimedEvent> events;

    Engine(KritOptions &options);
    ~Engine();

    JSValue &scriptContext() { return _scriptContext; }

    RenderContext &renderCtx() {
        if (phase != FramePhase::Render) {
            panic("the render context can only be accessed during the render "
                  "phase");
        }
        return ctx;
    }

    void run();
    bool doFrame();

    /**
     * Ends the run() loop.
     */
    void quit() { running = false; }

    /**
     * Current time in milliseconds, as a float with microsecond precision.
     */
    float time();

    Color bgColor = Color::black();

    void *userData = nullptr;

    std::vector<Camera> cameras;

    std::string cursor;

    bool useSystemFullScreen{false};

    void update();
    void fixedUpdate();
    void render();

    void setTimeout(TimedEvent::SignalType s, float delay = 0,
                    void *userData = nullptr);

    void addCursor(const std::string &cursorPath, const std::string &cursor,
                   int resolution = 0, int x = 0, int y = 0);
    void setCursor(const std::string &cursor);

    Camera &addCamera();

    template <typename T> T *data() { return static_cast<T *>(this->userData); }

#define DECLARE_ASSET_GETTER(N, T)                                             \
    std::shared_ptr<T> get##N(const std::string &s) { return assets.get<T>(s); }
    DECLARE_ASSET_GETTER(Image, ImageData)
    DECLARE_ASSET_GETTER(Atlas, TextureAtlas)
    DECLARE_ASSET_GETTER(Font, Font)
    DECLARE_ASSET_GETTER(Spine, SpineData)
    DECLARE_ASSET_GETTER(Sound, SoundData)
    DECLARE_ASSET_GETTER(Music, MusicData)
    DECLARE_ASSET_GETTER(Text, std::string)
    DECLARE_ASSET_GETTER(Particle, ParticleEffect)
#undef DECLARE_ASSET_GETTER

private:
    std::chrono::steady_clock clock;
    std::chrono::steady_clock::time_point appStart, frameStart, frameFinish;
    // accumulator and total elapsed time, in microseconds
    int32_t accumulator = 0, elapsed = 0;
    // microseconds per frame, at framerate and framerate+2
    int32_t frameDelta, frameDelta2;
    TaskManager *taskManager = nullptr;
    SDL_Cursor *_cursor = nullptr;
    JSValue _scriptContext = JS_UNDEFINED;

    void handleEvents();
    void cleanup();
    void chooseCursor();

    friend struct Editor;
    friend struct Renderer;
};

RenderContext &render();

/** these hooks are defined by the application */
void gameOptions(KritOptions &);
void gameBootstrap(Engine &);

}

#endif