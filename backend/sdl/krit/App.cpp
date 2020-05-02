#include "krit/App.h"
#include "krit/RenderThread.h"

#include <krit/input/Mouse.h>
#include "krit/TaskManager.h"
#include "SDL2/SDL.h"
#include <chrono>
#include <cmath>

namespace krit {

App::App(KritOptions &options):
    backend(options.title, options.width, options.height),
    window(options.width, options.height) {}

void App::run() {
    double frameDelta1 = 1.0 / (FPS - 1);
    double frameDelta2 = 1.0 / (FPS + 4);

    UpdateContext update;
    update.app = this;
    update.engine = &this->engine;
    update.window = &this->window;
    update.asset = &this->engine.asset;
    update.camera = &this->engine.camera;
    update.controls = &this->engine.controls;
    update.input = &this->engine.input;

    RenderContext render;
    render.app = this;
    render.engine = &this->engine;
    render.window = &this->window;
    render.drawCommandBuffer = &this->renderer.drawCommandBuffer;
    render.userData = this->engine.userData;

    double accumulator = 0, elapsed;
    std::chrono::steady_clock clock;
    auto frameStart = clock.now();
    auto frameFinish = frameStart;
    int cores = SDL_GetCPUCount();

    TaskManager taskManager(update, max(2, cores - 2));
    RenderThread renderThread(update, render, taskManager, backend.window);

    invoke(engine.onBegin, &update);

    this->running = true;
    while (this->running) {
        TaskManager::work(taskManager.mainQueue, update);
        do {
            frameFinish = clock.now();
            elapsed = std::chrono::duration_cast<std::chrono::microseconds>(frameFinish - frameStart).count() / 1000000.0;
        } while (elapsed < frameDelta2);
        accumulator += elapsed;
        update.elapsed = update.frameCount = 0;
        while (accumulator >= frameDelta2 && update.frameCount < MAX_FRAMES) {
            ++update.frameCount;
            update.elapsed += frameDelta1;
            accumulator -= frameDelta1;
            if (accumulator < 0) {
                accumulator = 0;
            }
        }
        update.frameId += update.frameCount;
        render.elapsed = update.elapsed;
        render.frameId = update.frameId;
        if (accumulator > frameDelta2) {
            accumulator = fmod(accumulator, frameDelta2);
        }
        frameStart = frameFinish;
        this->engine.controls.reset();
        this->engine.reset();
        this->handleEvents(update);
        this->engine.update(update);
        if (engine.finished) {
            this->quit();
        }

        if (update.frameCount > 0) {
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
                    this->window.setTo(w, h);
                }
                break;
            }
            case SDL_KEYDOWN: {
                context.controls->key.registerDown(static_cast<Key>(event.key.keysym.scancode));
                break;
            }
            case SDL_KEYUP: {
                context.controls->key.registerUp(static_cast<Key>(event.key.keysym.scancode));
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                context.controls->mouse.registerDown(sdlMouseButton(event.button.button));
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                context.controls->mouse.registerUp(sdlMouseButton(event.button.button));
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

void App::setFullScreen(bool full) {
    if (this->full != full) {
        this->backend.setFullScreen(this->full = full);
    }
}

}
