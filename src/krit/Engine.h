#ifndef KRIT_ENGINE
#define KRIT_ENGINE

#include "krit/Camera.h"
#include "krit/Options.h"
#include "krit/Window.h"
#include "krit/asset/AssetCache.h"
#include "krit/input/InputContext.h"
#include "krit/io/Io.h"
#include "krit/math/Dimensions.h"
#include "krit/platform/Platform.h"
#include "krit/render/Renderer.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"
#include "krit/utils/Signal.h"
#if KRIT_ENABLE_TEXT
#include "krit/asset/Font.h"
#endif
#if KRIT_ENABLE_AUDIO
#include "krit/audio/AudioBackend.h"
#endif
#if KRIT_ENABLE_NET
#include "krit/net/Net.h"
#endif
#if KRIT_ENABLE_SCRIPT
#include "krit/script/ScriptEngine.h"
#endif
#include <chrono>
#include <list>
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
struct AudioData;
struct SpineData;

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

    struct Scope {
        Scope(Engine *);
        ~Scope();
        Engine *prev{nullptr};
    };

    Scope scope() { return Scope(this); }

public:
    Io *io = ioFile;

    // backends
    std::unique_ptr<Platform> platform;
#if KRIT_ENABLE_NET
    std::unique_ptr<Net> net;
#endif

    RenderContext ctx;
    Window window;
    Renderer renderer;
    InputContext input;
    AssetCache assets;

#if KRIT_ENABLE_TEXT
    FontManager fonts;
#endif
#if KRIT_ENABLE_AUDIO
    AudioBackend audio;
#endif
#if KRIT_ENABLE_SCRIPT
    ScriptEngine script;
#endif

#if KRIT_ENABLE_CURSORS
    std::unordered_map<std::string, std::vector<std::pair<int, SDL_Cursor *>>>
        cursors;
#endif

    FramePhase phase = FramePhase::Inactive;
    int fixedFramerate{0};
    bool running{false};
    bool paused{false};
    bool fixedFrameRate{false};
    float speed{1};
    double totalElapsed{0};

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

#if KRIT_ENABLE_SCRIPT
    JSValue scriptContext{JS_UNDEFINED};
#endif

    RenderContext &renderCtx() {
        if (phase != FramePhase::Render) {
            panic("the render context can only be accessed during the render "
                  "phase");
        }
        return ctx;
    }

    void start();
    bool runFrame();

    void run();

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

    bool block{true};

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
    DECLARE_ASSET_GETTER(Audio, AudioData)
    DECLARE_ASSET_GETTER(Text, std::string)
    DECLARE_ASSET_GETTER(Particle, ParticleEffect)
#undef DECLARE_ASSET_GETTER

    std::unique_ptr<TaskManager> taskManager;

private:
    std::chrono::steady_clock clock;
    std::chrono::steady_clock::time_point appStart, frameStart, frameFinish;
    // accumulator and total elapsed time, in microseconds
    int32_t accumulator = 0, elapsed = 0;
    // microseconds per frame, at framerate and framerate+2
    int32_t frameDelta, frameDelta2;
#if KRIT_ENABLE_CURSORS
    SDL_Cursor *_cursor = nullptr;
#endif

    void handleEvents();
    void handleEvent(SDL_Event &);
    void cleanup();

#if KRIT_ENABLE_CURSORS
    void chooseCursor();
#endif

    friend struct Editor;
    friend struct Renderer;
};

RenderContext &render();

/** these hooks are defined by the application */
void gameOptions(KritOptions &);
void gameBootstrap(Engine &);

}

#endif