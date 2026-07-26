#include "krit/Window.h"
#include "krit/Engine.h"
#include "krit/TaskManager.h"
#include "krit/render/Gl.h"
#include "krit/utils/Panic.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>

namespace krit {

Window::Window(KritOptions &options)
    : fullScreenDimensions(options.fullscreenWidth, options.fullscreenHeight) {
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        panic("SDL init failed: %s", SDL_GetError());
    }

#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    if (options.windowProperties) {
        SDL_SetBooleanProperty(options.windowProperties,
                               SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
        if (!options.windowBorder) {
            SDL_SetBooleanProperty(options.windowProperties,
                                   SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN,
                                   true);
        }
        window = SDL_CreateWindowWithProperties(options.windowProperties);
    } else {
        SDL_WindowFlags flags = SDL_WINDOW_OPENGL;
        if (!options.windowBorder) {
            flags |= SDL_WINDOW_BORDERLESS;
        }
        window = SDL_CreateWindow(options.title.c_str(), options.width,
                                  options.height, flags);
    }
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

    float x, y;
    int wx, wy;
    SDL_GetGlobalMouseState(&x, &y);
    SDL_GetWindowPosition(this->window, &wx, &wy);
    SDL_WarpMouseInWindow(this->window, x - wx, y - wy);
    SDL_StopTextInput(window);
}

Window::~Window() {
    if (this->glContext) {
        SDL_GL_DestroyContext(this->glContext);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
}

void Window::setFullScreen(bool full) {
    engine->taskManager->pushMain([=]() {
        if (this->full != full) {
            if ((this->full = full)) {
                // SDL_DisplayMode mode;
                // SDL_GetDesktopDisplayMode(0, &mode);
                // if (fullScreenDimensions.x > 0 &&
                //     fullScreenDimensions.y > 0) {
                //     mode.w = fullScreenDimensions.x;
                //     mode.h = fullScreenDimensions.y;
                // }
                // SDL_SetWindowDisplayMode(window, &mode);
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            } else {
                SDL_SetWindowFullscreen(window, 0);
            }
            size();
            int x = this->x / 2, y = this->y / 2;
            SDL_WarpMouseInWindow(window, x, y);
        }
    });
}

void Window::show() { SDL_ShowWindow(window); }

void Window::hide() { SDL_HideWindow(window); }

}
