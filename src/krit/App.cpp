#include "krit/App.h"
#include "krit/asset/AssetContext.h"
#include "krit/input/InputContext.h"
#include "krit/input/Mouse.h"
#include "krit/TaskManager.h"
#include "krit/utils/Panic.h"
#include "krit/editor/Editor.h"
#include <SDL.h>
#include <chrono>
#include <cmath>
#include <csignal>
#include "imgui_impl_sdl.h"

namespace krit {

static App *current;

void sigintHandler(int sig_num) { 
    current->quit();
} 

App::App(KritOptions &options):
    dimensions(options.width, options.height),
    framerate(options.framerate),
    fixedFramerate(options.fixedFramerate) {}

void App::run() {
    current = this;

    std::signal(SIGINT, sigintHandler);

    // SDL
    SDL_Init(SDL_INIT_VIDEO);

    // SDL_Image
    IMG_Init(IMG_INIT_PNG);

    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        dimensions.width(), dimensions.height(),
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        panic(SDL_GetError());
    }

    surface = SDL_GetWindowSurface(window);

    checkForGlErrors("SDL init");

    double frameDelta = 1.0 / fixedFramerate;
    double frameDelta2 = 1.0 / (fixedFramerate + 1);
    
    // base context structs
    AssetContext asset(engine.assetCache);
    InputContext input;

    // the RenderContext will be upcast to an UpdateContext during the update phase
    RenderContext ctx;
    ctx.app = this;
    ctx.engine = &engine;
    ctx.window = &dimensions;
    ctx.asset = &engine.asset;
    ctx.camera = &engine.camera;
    ctx.drawCommandBuffer = &renderer.drawCommandBuffer;
    ctx.userData = engine.userData;

    UpdateContext *update = &ctx;

    double accumulator = 0, elapsed;
    std::chrono::steady_clock clock;
    auto frameStart = clock.now();
    auto frameFinish = frameStart;
    // bool lockFramerate = true;

    TaskManager taskManager(ctx, 3);
    renderer.init(window);

    invoke(engine.onBegin, update);

    running = true;
    while (running) {
        TaskManager::work(taskManager.mainQueue, *update);
        TaskManager::work(taskManager.renderQueue, ctx);
        // do {
        frameFinish = clock.now();
        elapsed = std::chrono::duration_cast<std::chrono::microseconds>(frameFinish - frameStart).count() / 1000000.0 * engine.speed;
        // } while (lockFramerate && elapsed < frameDelta2);
        accumulator += elapsed;
        ctx.elapsed = ctx.frameCount = 0;

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

        SDL_LockMutex(renderer.renderMutex);
        engine.render(ctx);
        renderer.renderFrame(ctx);
        SDL_UnlockMutex(renderer.renderMutex);
    }

    TaskManager::instance->killed = true;
    exit(0);
}

MouseButton sdlMouseButton(int b) {
    switch (b) {
        case SDL_BUTTON_MIDDLE: return MouseMiddle;
        case SDL_BUTTON_RIGHT: return MouseRight;
        default: return MouseLeft;
    }
}

void App::handleEvents() {
    static bool seenMouseEvent = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        bool handleKey = true,
            handleMouse = true;
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
                        int w = event.window.data1,
                            h = event.window.data2;
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
                        engine.input.keyDown(static_cast<Key>(event.key.keysym.scancode));
                    }
                }
                break;
            }
            case SDL_KEYUP: {
                if (handleKey) {
                    engine.input.keyUp(static_cast<Key>(event.key.keysym.scancode));
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
                seenMouseEvent = true;
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
    SDL_SetWindowFullscreen(this->window, full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    this->full = full;
    SDL_WarpMouseInWindow(this->window, this->dimensions.width()/2, this->dimensions.height()/2);
}

}
