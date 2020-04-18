#include "krit/App.h"

#include <krit/input/Mouse.h>
#include <chrono>
#include <cmath>

namespace krit {

#ifndef SINGLE_THREAD

struct RenderThreadData {
    App *app;
    SDL_mutex *renderMutex;
    SDL_mutex *renderCondMutex;
    SDL_cond *renderCond;
    RenderContext *context;
    bool killed = false;

    RenderThreadData(App* app, SDL_mutex* renderMutex, SDL_mutex* renderCondMutex, SDL_cond* renderCond, RenderContext* context) :
        app(app),
        renderMutex(renderMutex),
        renderCondMutex(renderCondMutex),
        renderCond(renderCond),
        context(context) {}
};

App::App(KritOptions &options):
    backend(options.title, options.width, options.height),
    window(options.width, options.height) {}

int App::renderLoop(void *raw) {
    RenderThreadData *data = static_cast<RenderThreadData *>(raw);
    while (true) {
        SDL_LockMutex(data->renderCondMutex);
        SDL_CondWait(data->renderCond, data->renderCondMutex);
        SDL_UnlockMutex(data->renderCondMutex);

        if (data->killed) {
            SDL_DestroyCond(data->renderCond);
            SDL_DestroyMutex(data->renderMutex);
            SDL_DestroyMutex(data->renderCondMutex);
            return 0;
        }
        SDL_LockMutex(data->renderMutex);
        SDL_GL_MakeCurrent(data->app->backend.window, data->app->backend.glContext);
        data->app->flushRender(*data->context);
        SDL_UnlockMutex(data->renderMutex);
    }
}
#endif

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
    clock_t frameStart = clock(), frameFinish;

    #ifndef SINGLE_THREAD
    RenderThreadData renderThreadData(
        this,
        SDL_CreateMutex(),
        SDL_CreateMutex(),
        SDL_CreateCond(),
        &render
    );
    SDL_Thread *renderThread = SDL_CreateThread(App::renderLoop, "render", &renderThreadData);
    #endif

    invoke(engine.onBegin, &update);

    this->running = true;
    while (this->running) {
        do {
            frameFinish = clock();
            elapsed = static_cast<double>(frameFinish - frameStart) / CLOCKS_PER_SEC;
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
        this->engine.controls.clear();
        this->engine.controls.reset();
        this->engine.reset();
        this->handleEvents(update);
        this->engine.update(update);
        if (engine.finished) {
            this->quit();
        }

        if (update.frameCount > 0) {
            #ifndef SINGLE_THREAD
            SDL_LockMutex(renderThreadData.renderMutex);
            #endif

            this->renderer.startFrame(render);
            this->engine.render(render);

            #ifdef SINGLE_THREAD
            this->flushRender(render);
            #else
            SDL_UnlockMutex(renderThreadData.renderMutex);
            SDL_LockMutex(renderThreadData.renderCondMutex);
            SDL_CondSignal(renderThreadData.renderCond);
            SDL_UnlockMutex(renderThreadData.renderCondMutex);
            SDL_GL_MakeCurrent(this->backend.window, this->backend.glContext);
            #endif
        }
    }

    #ifndef SINGLE_THREAD
    SDL_LockMutex(renderThreadData.renderMutex);
    renderThreadData.killed = true;
    SDL_LockMutex(renderThreadData.renderCondMutex);
    SDL_CondSignal(renderThreadData.renderCond);
    SDL_UnlockMutex(renderThreadData.renderCondMutex);
    SDL_UnlockMutex(renderThreadData.renderMutex);
    SDL_WaitThread(renderThread, nullptr);
    #endif
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
