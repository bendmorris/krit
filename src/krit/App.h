#ifndef KRIT_APP
#define KRIT_APP

#include "krit/Engine.h"
#include "krit/io/Io.h"
#include "krit/math/Dimensions.h"
#include "krit/platform/Platform.h"
#include "krit/render/Renderer.h"
#include <chrono>
#include <string>

namespace krit {
struct KritOptions;
struct RenderContext;
struct TaskManager;

const int MAX_FRAMES = 5;
const int FPS = 60;

struct App;

extern App *app;

struct App {
    static RenderContext ctx;

private:
    struct AppScope {
        AppScope(App *);
        ~AppScope();
    };

    AppScope _scope;

public:
    // backends
    std::unique_ptr<Io> io;
    std::unique_ptr<Platform> platform;

    Engine engine;
    int framerate;
    int fixedFramerate;
    bool running = false;

    App(KritOptions &options);
    ~App();

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

private:
    std::chrono::steady_clock clock;
    std::chrono::steady_clock::time_point appStart, frameStart, frameFinish;
    // accumulator and total elapsed time, in microseconds
    int32_t accumulator = 0, elapsed = 0;
    // microseconds per frame, at framerate and framerate+2
    int32_t frameDelta, frameDelta2;
    TaskManager *taskManager = nullptr;

    void handleEvents();
    void cleanup();

    friend struct Editor;
    friend struct Renderer;
};

}

#endif