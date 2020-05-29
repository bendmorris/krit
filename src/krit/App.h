#ifndef KRIT_APP
#define KRIT_APP

#include "krit/render/RenderContext.h"
#include "krit/render/Renderer.h"
#include "krit/Engine.h"
#include "krit/Options.h"
#include "krit/UpdateContext.h"
#include <SDL.h>
#include <SDL_image.h>

namespace krit {

const int MAX_FRAMES = 5;
const int FPS = 60;

struct Editor;

struct App {
    std::string title;
    Engine engine;
    IntDimensions dimensions;
    Renderer renderer;
    // AudioBackend audio;

    App(KritOptions &options);

    ~App() {
        SDL_DestroyWindow(window);
    }

    /**
     * Run the app; will continue until
     */
    void run();

    /**
     * Ends the run() loop.
     */
    void quit() {
        running = false;
    }

    void setFullScreen(bool full) {
        SDL_SetWindowFullscreen(this->window, full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        this->full = full;
    }
    bool isFullScreen() { return full; }

    void getWindowSize(int *w, int *h) {
        SDL_GetWindowSize(this->window, w, h);
    }

    private:
        SDL_Window *window;
        SDL_Surface *surface;
        bool running = false;
        bool full = false;

        void handleEvents(UpdateContext&);

    friend struct Editor;
    friend struct Renderer;
};

}

#endif