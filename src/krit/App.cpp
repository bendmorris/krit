#include "krit/App.h"
#include "krit/RenderThread.h"
#include "krit/input/Mouse.h"
#include "krit/TaskManager.h"
#include "krit/utils/Panic.h"
#include "krit/editor/Editor.h"
#include <SDL.h>
#include <chrono>
#include <cmath>
#include "imgui_impl_sdl.h"

namespace krit {

App::App(KritOptions &options):
    dimensions(options.width, options.height) {}

void App::run() {
    // SDL
    SDL_Init(SDL_INIT_VIDEO);

    // SDL_Image
    IMG_Init(IMG_INIT_PNG);

    this->window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        dimensions.width(), dimensions.height(),
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!this->window) {
        panic(SDL_GetError());
    }

    this->surface = SDL_GetWindowSurface(this->window);

    checkForGlErrors("SDL init");

    double frameDelta = 1.0 / FPS;
    double frameDelta2 = 1.0 / (FPS + 1);

    UpdateContext update;
    update.app = this;
    update.engine = &this->engine;
    update.window = &this->dimensions;
    update.asset = &this->engine.asset;
    update.camera = &this->engine.camera;
    update.controls = &this->engine.controls;
    update.input = &this->engine.input;

    RenderContext render;
    render.app = this;
    render.engine = &this->engine;
    render.window = &this->dimensions;
    render.drawCommandBuffer = &this->renderer.drawCommandBuffer;

    double accumulator = 0, elapsed;
    std::chrono::steady_clock clock;
    auto frameStart = clock.now();
    auto frameFinish = frameStart;
    bool lockFramerate = true;
    int cores = SDL_GetCPUCount();

    TaskManager taskManager(update, std::max(2, cores - 2));
    RenderThread renderThread(update, render, taskManager, window);

    invoke(engine.onBegin, &update);

    this->running = true;
    while (this->running) {
        TaskManager::work(taskManager.mainQueue, update);
        do {
            frameFinish = clock.now();
            elapsed = std::chrono::duration_cast<std::chrono::microseconds>(frameFinish - frameStart).count() / 1000000.0;
        } while (lockFramerate && elapsed < frameDelta2);
        accumulator += elapsed;
        update.elapsed = update.frameCount = 0;

        this->engine.controls.reset();
        this->engine.reset();
        this->handleEvents(update);

        while (accumulator >= frameDelta2 && update.frameCount < MAX_FRAMES) {
            accumulator -= frameDelta;
            if (accumulator < 0) {
                accumulator = 0;
            }
            ++update.frameCount;
            ++update.frameId;
            render.elapsed += frameDelta;
            render.frameId = update.frameId;
            
            this->engine.fixedUpdate(update);
        }
        if (accumulator > frameDelta2) {
            accumulator = fmod(accumulator, frameDelta2);
        }

        update.elapsed = render.elapsed = elapsed;
        this->engine.update(update);
        if (engine.finished) {
            this->quit();
        }
        frameStart = frameFinish;

        SDL_LockMutex(renderThread.renderMutex);

        this->engine.render(render);

        SDL_UnlockMutex(renderThread.renderMutex);
        SDL_LockMutex(renderThread.renderCondMutex);
        SDL_CondSignal(renderThread.renderCond);
        SDL_UnlockMutex(renderThread.renderCondMutex);

        SDL_LockMutex(renderThread.renderCondMutex);
        SDL_CondWaitTimeout(renderThread.renderCond, renderThread.renderCondMutex, frameDelta2 * 1000);
        SDL_UnlockMutex(renderThread.renderCondMutex);
    }

    SDL_LockMutex(renderThread.renderMutex);
    renderThread.killed = true;
    // TODO: kill workers
    SDL_LockMutex(renderThread.renderCondMutex);
    SDL_CondSignal(renderThread.renderCond);
    SDL_UnlockMutex(renderThread.renderCondMutex);
    SDL_UnlockMutex(renderThread.renderMutex);
    SDL_WaitThread(renderThread.thread, nullptr);
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
                this->quit();
                break;
            }
            case SDL_WINDOWEVENT: {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    // resize
                    int w = event.window.data1,
                        h = event.window.data2;
                    this->dimensions.setTo(w, h);
                }
                break;
            }
            case SDL_KEYDOWN: {
                if (handleKey) {
                    context.controls->key.registerDown(static_cast<Key>(event.key.keysym.scancode));
                }
                break;
            }
            case SDL_KEYUP: {
                if (handleKey) {
                    context.controls->key.registerUp(static_cast<Key>(event.key.keysym.scancode));
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if (handleMouse) {
                    context.controls->mouse.registerDown(sdlMouseButton(event.button.button));
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if (handleMouse) {
                    context.controls->mouse.registerUp(sdlMouseButton(event.button.button));
                }
                break;
            }
            case SDL_MOUSEMOTION: {
                context.controls->mouse.registerPos(event.motion.x, event.motion.y);
                break;
            }
            case SDL_WINDOWEVENT_ENTER: {
                context.controls->mouse.registerOver(true);
                break;
            }
            case SDL_WINDOWEVENT_LEAVE: {
                context.controls->mouse.registerOver(false);
                break;
            }
        }
    }
}

}
