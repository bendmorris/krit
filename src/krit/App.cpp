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
    InputContext input(engine.controls);

    // the RenderContext will be upcast to an UpdateContext during the update phase
    RenderContext ctx;
    ctx.app = this;
    ctx.engine = &engine;
    ctx.window = &dimensions;
    ctx.asset = &asset;
    ctx.camera = &engine.camera;
    ctx.input = &input;
    ctx.drawCommandBuffer = &renderer.drawCommandBuffer;

    UpdateContext *update = &ctx;
    update->userData = engine.userData;

    double accumulator = 0, elapsed;
    std::chrono::steady_clock clock;
    auto frameStart = clock.now();
    auto frameFinish = frameStart;
    // bool lockFramerate = true;
    int cores = SDL_GetCPUCount();

    TaskManager taskManager(ctx, std::min(4, std::max(2, cores - 2)));
    renderer.init(window);

    invoke(engine.onBegin, update);

    running = true;
    while (running) {
        TaskManager::work(taskManager.mainQueue, *update);
        TaskManager::work(taskManager.renderQueue, ctx);
        // do {
        frameFinish = clock.now();
        elapsed = std::chrono::duration_cast<std::chrono::microseconds>(frameFinish - frameStart).count() / 1000000.0;
        // } while (lockFramerate && elapsed < frameDelta2);
        accumulator += elapsed;
        ctx.elapsed = ctx.frameCount = 0;

        engine.reset();
        handleEvents(ctx);

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
        input.update(ctx);
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
}

MouseButton sdlMouseButton(int b) {
    switch (b) {
        case SDL_BUTTON_MIDDLE: return MouseMiddle;
        case SDL_BUTTON_RIGHT: return MouseRight;
        default: return MouseLeft;
    }
}

void App::handleEvents(UpdateContext &context) {
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
                }
                break;
            }
            case SDL_KEYDOWN: {
                if (handleKey) {
                    engine.controls.key.registerDown(static_cast<Key>(event.key.keysym.scancode));
                }
                break;
            }
            case SDL_KEYUP: {
                if (handleKey) {
                    engine.controls.key.registerUp(static_cast<Key>(event.key.keysym.scancode));
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if (handleMouse) {
                    engine.controls.mouse.registerDown(sdlMouseButton(event.button.button));
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if (handleMouse) {
                    engine.controls.mouse.registerUp(sdlMouseButton(event.button.button));
                }
                break;
            }
            case SDL_MOUSEMOTION: {
                engine.controls.mouse.registerPos(event.motion.x, event.motion.y);
                break;
            }
            case SDL_WINDOWEVENT_ENTER: {
                engine.controls.mouse.registerOver(true);
                break;
            }
            case SDL_WINDOWEVENT_LEAVE: {
                engine.controls.mouse.registerOver(false);
                break;
            }
        }
    }
}

void App::setFullScreen(bool full) {
    SDL_SetWindowFullscreen(this->window, full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    this->full = full;
}

}
