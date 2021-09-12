#ifndef KRIT_APP
#define KRIT_APP

#include "SDL_surface.h"
#include "SDL_video.h"
#include "krit/Engine.h"
#include "krit/math/Dimensions.h"
#include "krit/render/Renderer.h"
#include "krit/sound/AudioBackend.h"
#include <string>

namespace krit {
struct KritOptions;
struct RenderContext;

const int MAX_FRAMES = 5;
const int FPS = 60;

struct App {
    static RenderContext ctx;

    std::string title;
    Engine engine;
    IntDimensions dimensions;
    IntDimensions fullScreenDimensions;
    Renderer renderer;
    AudioBackend audio;
    int framerate;
    int fixedFramerate;

    App(KritOptions &options);

    ~App() {
        if (window) {
            SDL_DestroyWindow(window);
        }
    }

    /**
     * Run the app; will continue until
     */
    [[noreturn]] void run();

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
    bool running = false;
    bool full = false;
    bool startFullscreen = false;

    void handleEvents();

    friend struct Editor;
    friend struct Renderer;
};

}

#endif