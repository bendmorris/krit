#include "krit/Window.h"
#include "krit/Engine.h"
#include "krit/TaskManager.h"
#include "krit/render/Gl.h"
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

#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
#if KRIT_ENABLE_MULTISAMPLING
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, GL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, GL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, GL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    window = SDL_CreateWindow(
        options.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        options.width, options.height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (!window) {
        panic("SDL_CreateWindow failed: %s", SDL_GetError());
    }
    this->glContext = SDL_GL_CreateContext(window);
    if (!this->glContext) {
        panic("SDL_GL_CreateContext failed: %s", SDL_GetError());
    }
    SDL_SetWindowSize(window, options.width, options.height);

    if (options.fullscreen) {
        this->setFullScreen(true);
    }

    int x, y, wx, wy;
    SDL_GetGlobalMouseState(&x, &y);
    SDL_GetWindowPosition(this->window, &wx, &wy);
    SDL_WarpMouseInWindow(this->window, x - wx, y - wy);
    
    SDL_StopTextInput();
}

Window::~Window() {
    if (this->glContext) {
        SDL_GL_DeleteContext(this->glContext);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void Window::setFullScreen(bool full) {
    TaskManager::instance->pushMain([=]() {
        if (this->full != full) {
            if ((this->full = full)) {
                // SDL_DisplayMode mode;
                // SDL_GetDesktopDisplayMode(0, &mode);
                // if (fullScreenDimensions.x() > 0 &&
                //     fullScreenDimensions.y() > 0) {
                //     mode.w = fullScreenDimensions.x();
                //     mode.h = fullScreenDimensions.y();
                // }
                // SDL_SetWindowDisplayMode(window, &mode);
                SDL_SetWindowFullscreen(window, engine->useSystemFullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP);
            } else {
                SDL_SetWindowFullscreen(window, 0);
            }
            size();
            int x = this->x() / 2, y = this->y() / 2;
            SDL_WarpMouseInWindow(window, x, y);
        }
    });
}

}
