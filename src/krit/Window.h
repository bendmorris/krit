#ifndef KRIT_WINDOW
#define KRIT_WINDOW

#include "krit/Options.h"
#include "krit/math/Dimensions.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

namespace krit {

struct App;
struct Editor;
struct Engine;
struct Renderer;

struct Window : public IntDimensions {
    IntDimensions &size() {
        int _x = x(), _y = y();
        getWindowSize(&x(), &y());
        if (x() != _x || y() != _y) {
            skipFrames = 3;
        }
        return *this;
    }

    bool full = false;

    Window(KritOptions &options);

    ~Window() {
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    }

    void setFullScreen(bool full);
    bool isFullScreen() { return full; }

    void getWindowSize(int *w, int *h) {
        SDL_GetWindowSize(this->window, w, h);
    }

private:
    SDL_Window *window = nullptr;
    SDL_Surface *surface = nullptr;
    IntDimensions fullScreenDimensions;
    int skipFrames = 0;

    friend struct App;
    friend struct Editor;
    friend struct Engine;
    friend struct Renderer;
};

}

#endif
