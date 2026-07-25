#ifndef KRIT_WINDOW
#define KRIT_WINDOW

#include "krit/Options.h"
#include "krit/math/Dimensions.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

namespace krit {

struct App;
struct Editor;
struct Engine;
struct Renderer;

struct Window : public IntDimensions {
    int width() { return size().x; }
    int height() { return size().y; }

    IntDimensions &size() {
        getWindowSize(&x, &y);
        return *this;
    }

    bool full = false;

    Window(KritOptions &options);
    ~Window();

    void setFullScreen(bool full);
    bool isFullScreen() { return full; }

    void getWindowSize(int *w, int *h) {
        SDL_GetWindowSize(this->window, w, h);
    }

    void setWindowSize(int w, int h) {
        SDL_SetWindowSize(this->window, w, h);
        SDL_GetWindowSize(this->window, &x, &y);
    }

    void setWindowPos(int x, int y) {
        SDL_SetWindowPosition(window, x, y);
    }

    void show();
    void hide();

    SDL_Window *window = nullptr;

private:
    SDL_Surface *surface = nullptr;
    SDL_GLContext glContext;
    IntDimensions fullScreenDimensions;
    int skipFrames = 0;

    friend struct App;
    friend struct Editor;
    friend struct Engine;
    friend struct Renderer;
};

}

#endif
