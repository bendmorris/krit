#include "krit/App.h"
#if KRIT_ENABLE_TOOLS
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "krit/editor/Editor.h"
#endif
#include "krit/Options.h"
#include "krit/TaskManager.h"
#include "krit/asset/Font.h"
#include "krit/input/InputContext.h"
#include "krit/input/Key.h"
#include "krit/input/Mouse.h"
#include "krit/render/Gl.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Panic.h"
#include "krit/utils/Signal.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_mutex.h>
#include <cmath>
#include <csignal>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#if TRACY_ENABLE
#include "krit/tracy/Tracy.hpp"
#endif

namespace krit {
struct UpdateContext;

App *app {0};
RenderContext App::ctx;

void sigintHandler(int) { App::ctx.app->quit(); }

void segvHandler(int sig) {
    void *array[10];
    size_t size = backtrace(array, 10);
    printf("Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDOUT_FILENO);
    exit(1);
}

#ifdef __EMSCRIPTEN__
static void __doFrame(void *app) { ((App *)app)->doFrame(); }
#endif

App::App(KritOptions &options)
    : _scope(this), io(krit::io()), platform(krit::platform()),
#ifdef __EMSCRIPTEN__
      engine((emscripten_set_main_loop_arg(__doFrame, this, 0, 0), options)),
#else
      engine(options),
#endif
      framerate(options.framerate), fixedFramerate(options.fixedFramerate) {
}

App::~App() {}

App::AppScope::AppScope(App *app) {
    if (krit::app) {
        panic("can only have a single App running at once");
    }
    krit::app = App::ctx.app = app;
}

App::AppScope::~AppScope() { krit::app = App::ctx.app = nullptr; }

float App::time() {
    return std::chrono::duration_cast<std::chrono::microseconds>(clock.now() -
                                                                 appStart)
               .count() /
           1000.0f;
}

void App::run() {
    static bool registeredHandlers = false;
    if (!registeredHandlers) {
#ifndef __EMSCRIPTEN
        registeredHandlers = true;
        std::signal(SIGINT, sigintHandler);
        std::signal(SIGSEGV, segvHandler);
#endif
    }

    appStart = clock.now();

    // // SDL_Image
    // int flags = IMG_INIT_PNG;
    // int result = IMG_Init(flags);
    // if ((result & flags) != flags) {
    //     panic("PNG support is required");
    // }

    frameDelta = 1000000 / fixedFramerate;
    frameDelta2 = 1000000 / (fixedFramerate + 2);

    // the RenderContext will be upcast to an UpdateContext during the update
    // phase
    ctx.engine = &engine;
    ctx.window = &engine.window;
    ctx.camera = nullptr;
    ctx.drawCommandBuffer = &engine.renderer.drawCommandBuffer;
    ctx.audio = &engine.audio;
    ctx.userData = engine.userData;

    UpdateContext *update = &ctx;

    frameStart = clock.now();
    frameFinish = frameStart;
    // bool lockFramerate = true;

    taskManager = new TaskManager(ctx, 3);

    // generate an initial MouseMotion event; without this, SDL will return an
    // invalid initial mouse position

    invoke(engine.onBegin, update);
    engine.onBegin = nullptr;

    running = true;
#ifndef __EMSCRIPTEN__
    while (running) {
        if (!doFrame()) {
            break;
        }
    }

    cleanup();
#endif
}

void App::cleanup() {
    invoke(engine.onEnd, &ctx);
    TaskManager::instance->cleanup();
}

bool App::doFrame() {
    UpdateContext *update = &ctx;
    ++ctx.tickId;

    TaskManager::work(taskManager->mainQueue, *update);
    TaskManager::work(taskManager->renderQueue, ctx);
    // do {
    frameFinish = clock.now();
    elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                  frameFinish - frameStart)
                  .count() *
              engine.speed;
    // } while (lockFramerate && elapsed < frameDelta2);
    // if (1.0 / elapsed < 50) {
    //     printf("%.2f\n", 1.0 / elapsed);
    // }
    accumulator += elapsed;
    ctx.elapsed = ctx.frameCount = 0;
    engine.elapsed += elapsed / 1000000.0;

    engine.input.startFrame();
    handleEvents();
    engine.input.endFrame();

    if (engine.finished) {
        quit();
        return false;
    }

    while (accumulator >= frameDelta2 && ctx.frameCount < MAX_FRAMES) {
        accumulator -= frameDelta;
        if (accumulator < 0) {
            accumulator = 0;
        }
        ++ctx.frameCount;
        ++ctx.frameId;
        ctx.elapsed = frameDelta / 1000000.0;
        engine.fixedUpdate(ctx);
    }
    if (accumulator > frameDelta2) {
        accumulator = fmod(accumulator, frameDelta2);
    }

    ctx.elapsed = elapsed / 1000000.0;
    engine.update(ctx);
    if (engine.finished) {
        quit();
        return false;
    }
    frameStart = frameFinish;

    engine.render(ctx);
    engine.flip(ctx);

#if TRACY_ENABLE
    FrameMark;
#endif

    return true;
}

MouseButton sdlMouseButton(int b) {
    switch (b) {
        case SDL_BUTTON_MIDDLE:
            return MouseMiddle;
        case SDL_BUTTON_RIGHT:
            return MouseRight;
        default:
            return MouseLeft;
    }
}

void App::handleEvents() {
    static bool seenMouseEvent = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        bool handleKey = true, handleMouse = true;
#if KRIT_ENABLE_TOOLS
        if (Editor::imguiInitialized) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            auto &io = ImGui::GetIO();
            handleKey = !io.WantTextInput;
            handleMouse = !io.WantCaptureMouse;
        }
#endif
        switch (event.type) {
            case SDL_QUIT: {
                quit();
                break;
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_ENTER: {
                        engine.input.registerMouseOver(true);
                        break;
                    }
                    case SDL_WINDOWEVENT_LEAVE: {
                        engine.input.registerMouseOver(false);
                        break;
                    }
                    case SDL_WINDOWEVENT_CLOSE: {
                        engine.quit();
                        break;
                    }
                }
                break;
            }
            case SDL_KEYDOWN: {
                if (!event.key.repeat) {
                    if (handleKey) {
                        engine.input.keyDown(
                            static_cast<Key>(event.key.keysym.scancode));
                    }
                }
                break;
            }
            case SDL_KEYUP: {
                if (handleKey) {
                    engine.input.keyUp(
                        static_cast<Key>(event.key.keysym.scancode));
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if (handleMouse) {
                    engine.input.mouseDown(sdlMouseButton(event.button.button));
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if (handleMouse) {
                    engine.input.mouseUp(sdlMouseButton(event.button.button));
                }
                break;
            }
            case SDL_MOUSEMOTION: {
                if (event.motion.x || event.motion.y) {
                    seenMouseEvent = true;
                }
                break;
            }
            case SDL_MOUSEWHEEL: {
                if (handleMouse && event.wheel.y) {
                    engine.input.mouseWheel(
                        event.wheel.y *
                        (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? 1
                                                                         : -1));
                }
                break;
            }
        }
    }

    // the mouse position will return (0,0) if we haven't had any mouse events,
    // but since this is a valid position, we use (-1,-1) for no position;
    // therefore, we need to avoid asking for position until a SDL_MOUSEMOTION
    // event has been seen
    if (seenMouseEvent) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        engine.input.registerMousePos(mouseX, mouseY);
    }
}

}
