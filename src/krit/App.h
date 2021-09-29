#ifndef KRIT_APP
#define KRIT_APP

#include "krit/Engine.h"
#include "krit/math/Dimensions.h"
#include "krit/render/Renderer.h"
#include "krit/sound/AudioBackend.h"
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
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

    std::string title;
    IntDimensions dimensions;
    IntDimensions fullScreenDimensions;
    InputContext input;
    Engine engine;
    Renderer renderer;
    AudioBackend audio;
    int framerate;
    int fixedFramerate;
    bool running = false;

    App(KritOptions &options);

    ~App() {
        if (window) {
            SDL_DestroyWindow(window);
        }
    }

    void run();
    bool doFrame();

    /**
     * Ends the run() loop.
     */
    void quit() { running = false; }

    void setFullScreen(bool full);
    bool isFullScreen() { return full; }

    void getWindowSize(int *w, int *h) {
        SDL_GetWindowSize(this->window, w, h);
    }

private:
    SDL_Window *window;
    SDL_Surface *surface;
    bool full = false;
    bool startFullscreen = false;
    std::chrono::steady_clock clock;
    std::chrono::steady_clock::time_point frameStart, frameFinish;
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