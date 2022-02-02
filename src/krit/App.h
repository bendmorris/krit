#ifndef KRIT_APP
#define KRIT_APP

#include "krit/Engine.h"
#include "krit/math/Dimensions.h"
#include "krit/render/Renderer.h"
#include <chrono>
#include <string>

namespace krit {
struct KritOptions;
struct RenderContext;
struct TaskManager;

const int MAX_FRAMES = 5;
const int FPS = 60;

struct App {
    static RenderContext ctx;

    Engine engine;
    int framerate;
    int fixedFramerate;
    bool running = false;

    App(KritOptions &options);

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
    double accumulator = 0, elapsed;
    double frameDelta, frameDelta2;
    TaskManager *taskManager = nullptr;

    void handleEvents();
    void cleanup();

    friend struct Editor;
    friend struct Renderer;
};

}

#endif