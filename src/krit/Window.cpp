#include "krit/Window.h"
#include "krit/utils/Panic.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_video.h>

namespace krit {

Window::Window(KritOptions &options)
    : fullScreenDimensions(options.fullscreenWidth, options.fullscreenHeight) {
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    SDL_Init(SDL_INIT_VIDEO);

    window =
        SDL_CreateWindow(options.title.c_str(), SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, options.width, options.height,
                         SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL |
                             SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!window) {
        panic(SDL_GetError());
    }
    SDL_SetWindowSize(window, options.width, options.height);

    surface = SDL_GetWindowSurface(window);

    if (options.fullscreen) {
        this->setFullScreen(true);
    }

    int x, y, wx, wy;
    SDL_GetGlobalMouseState(&x, &y);
    SDL_GetWindowPosition(this->window, &wx, &wy);
    SDL_WarpMouseInWindow(this->window, x - wx, y - wy);
}

void Window::setFullScreen(bool full) {
    if (this->full != full) {
        if ((this->full = full)) {
            SDL_DisplayMode mode;
            SDL_GetDesktopDisplayMode(0, &mode);
            if (fullScreenDimensions.width() > 0 &&
                fullScreenDimensions.height() > 0) {
                mode.w = fullScreenDimensions.width();
                mode.h = fullScreenDimensions.height();
            }
            SDL_SetWindowDisplayMode(window, &mode);
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        } else {
            SDL_SetWindowFullscreen(window, 0);
        }
        int x = width() / 2, y = height() / 2;
        SDL_WarpMouseInWindow(window, x, y);
        skipFrames = 3;
    }
}

}
