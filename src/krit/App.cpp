#include "krit/App.h"

#include <SDL.h>
#include <chrono>
#include <cmath>
#include <csignal>
#include <stdlib.h>

#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_image.h"
#include "SDL_keyboard.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "krit/Options.h"
#include "krit/TaskManager.h"
#include "krit/asset/Font.h"
#include "krit/editor/Editor.h"
#include "krit/input/InputContext.h"
#include "krit/input/Key.h"
#include "krit/input/Mouse.h"
#include "krit/render/Gl.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Panic.h"
#include "krit/utils/Signal.h"

namespace krit {
struct UpdateContext;

static App *current;
RenderContext App::ctx;

void sigintHandler(int sig_num) { current->quit(); }

App::App(KritOptions &options)
    : dimensions(options.width, options.height),
      fullScreenDimensions(options.fullscreenWidth, options.fullscreenHeight),
      framerate(options.framerate), fixedFramerate(options.fixedFramerate),
      startFullscreen(options.fullscreen) {}

void App::run() {
    if (ctx.app) {
        panic("can only have a single App running at once");
    }
    current = this;

    std::signal(SIGINT, sigintHandler);

    // freetype
    Font::init();

    // SDL
    SDL_Init(SDL_INIT_VIDEO);

    // SDL_Image
    IMG_Init(IMG_INIT_PNG);

    window = SDL_CreateWindow(
        title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        dimensions.width(), dimensions.height(),
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        panic(SDL_GetError());
    }

    surface = SDL_GetWindowSurface(window);

    checkForGlErrors("SDL init");

    double frameDelta = 1.0 / fixedFramerate;
    double frameDelta2 = 1.0 / (fixedFramerate + 1);

    // base context structs
    InputContext input;

    // the RenderContext will be upcast to an UpdateContext during the update
    // phase
    ctx.app = this;
    ctx.engine = &engine;
    ctx.window = &dimensions;
    ctx.camera = &engine.camera;
    ctx.drawCommandBuffer = &renderer.drawCommandBuffer;
    ctx.audio = &audio;
    ctx.userData = engine.userData;

    UpdateContext *update = &ctx;

    double accumulator = 0, elapsed;
    std::chrono::steady_clock clock;
    auto frameStart = clock.now();
    auto frameFinish = frameStart;
    // bool lockFramerate = true;

    TaskManager taskManager(ctx, 3);
    renderer.init(window);

    if (startFullscreen) {
        this->setFullScreen(true);
    }

    // generate an initial MouseMotion event; without this, SDL will return an invalid initial mouse position
    int x, y, wx, wy;
    SDL_GetGlobalMouseState(&x, &y);
    SDL_GetWindowPosition(this->window, &wx, &wy);
    SDL_WarpMouseInWindow(this->window, x - wx, y - wy);

    invoke(engine.onBegin, update);

    running = true;
    while (running) {
        TaskManager::work(taskManager.mainQueue, *update);
        TaskManager::work(taskManager.renderQueue, ctx);
        // do {
        frameFinish = clock.now();
        elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                      frameFinish - frameStart)
                      .count() /
                  1000000.0 * engine.speed;
        // } while (lockFramerate && elapsed < frameDelta2);
        accumulator += elapsed;
        ctx.elapsed = ctx.frameCount = 0;
        engine.elapsed += elapsed;

        engine.input.startFrame();
        handleEvents();
        engine.input.endFrame();

        while (accumulator >= frameDelta2 && ctx.frameCount < MAX_FRAMES) {
            accumulator -= frameDelta;
            if (accumulator < 0) {
                accumulator = 0;
            }
            ++ctx.frameCount;
            ++ctx.frameId;
            ctx.elapsed = frameDelta;
            engine.fixedUpdate(ctx);
        }
        if (accumulator > frameDelta2) {
            accumulator = fmod(accumulator, frameDelta2);
        }

        ctx.elapsed = elapsed;
        engine.update(ctx);
        if (engine.finished) {
            quit();
        }
        frameStart = frameFinish;

        Font::commit();
        engine.render(ctx);
        renderer.renderFrame(ctx);

        Font::flush();
    }

    TaskManager::instance->killed = true;
    exit(0);
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
        if (Editor::imguiInitialized) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            auto &io = ImGui::GetIO();
            handleKey = !io.WantTextInput;
            handleMouse = !io.WantCaptureMouse;
        }
        switch (event.type) {
            case SDL_QUIT: {
                quit();
                break;
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        // resize
                        int w = event.window.data1, h = event.window.data2;
                        dimensions.setTo(w, h);
                        break;
                    }
                    case SDL_WINDOWEVENT_ENTER: {
                        engine.input.registerMouseOver(true);
                        break;
                    }
                    case SDL_WINDOWEVENT_LEAVE: {
                        engine.input.registerMouseOver(false);
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
                    engine.input.mouseWheel(event.wheel.y * (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? 1 : -1));
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

void App::setFullScreen(bool full) {
    if ((this->full = full)) {
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(0, &mode);
        if (fullScreenDimensions.width() > 0 &&
            fullScreenDimensions.height() > 0) {
            mode.w = fullScreenDimensions.width();
            mode.h = fullScreenDimensions.height();
        }
        SDL_SetWindowDisplayMode(this->window, &mode);
        SDL_SetWindowFullscreen(this->window, SDL_WINDOW_FULLSCREEN);
    } else {
        SDL_SetWindowFullscreen(this->window, 0);
    }
    int x = this->dimensions.width() / 2,
        y = this->dimensions.height() / 2;
    SDL_WarpMouseInWindow(this->window, x, y);
}

}
