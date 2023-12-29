#include "krit/Engine.h"
#if KRIT_ENABLE_TOOLS
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "krit/editor/Editor.h"
#endif
#include "krit/CrashHandler.h"
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
#include <stdlib.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#if TRACY_ENABLE
#include "tracy/Tracy.hpp"
#endif

namespace krit {

struct UpdateContext;

Engine *engine{0};

#ifdef __EMSCRIPTEN__
static void __doFrame() {
    if (engine->running) {
        engine->doFrame();
    }
}
#endif

Engine::Engine(KritOptions &options)
    : _scope(this), io(krit::io()), net(krit::net()),
      platform(krit::platform()), window(options), renderer(window),
      fixedFramerate(options.fixedFramerate) {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(__doFrame, 0, 0);
#endif
    script.userData = this;
    _scriptContext = JS_NewObject(script.ctx);
}

Engine::~Engine() { JS_FreeValue(script.ctx, _scriptContext); }

Engine::EngineScope::EngineScope(Engine *engine) {
    if (krit::engine) {
        panic("can only have a single Engine running at once");
    }
    krit::engine = engine;
}

Engine::EngineScope::~EngineScope() { krit::engine = nullptr; }

float Engine::time() {
    return std::chrono::duration_cast<std::chrono::microseconds>(clock.now() -
                                                                 appStart)
               .count() /
           1000.0f;
}

void Engine::run() {
    CrashHandler::init();

    appStart = clock.now();

    // SDL_Image
    int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    int result = IMG_Init(flags);
#ifdef __EMSCRIPTEN__
    (void)result;
#else
    if ((result & flags) != flags) {
        panic("PNG/JPEG support is required; png=%s jpg=%s",
              result & IMG_INIT_PNG ? "y" : "n",
              result & IMG_INIT_JPG ? "y" : "n");
    }
#endif

    frameDelta = 1000000 / fixedFramerate;
    frameDelta2 = 1000000 / (fixedFramerate + 2);

    ctx.camera = nullptr;
    ctx.drawCommandBuffer = &renderer.drawCommandBuffer;

    frameStart = clock.now();
    frameFinish = frameStart;
    // bool lockFramerate = true;

    taskManager = new TaskManager(ctx, 3);

    // generate an initial MouseMotion event; without this, SDL will return an
    // invalid initial mouse position

    phase = FramePhase::Begin;
    invoke(onBegin, &updateCtx());
    phase = FramePhase::Inactive;
    onBegin = nullptr;

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

void Engine::cleanup() {
    invoke(onEnd, &ctx);
    TaskManager::instance->cleanup();
}

bool Engine::doFrame() {
    if (!running) {
        return false;
    }

    UpdateContext *update = &ctx;
    ++ctx.tickId;

    phase = FramePhase::Update;
    // do {
    frameFinish = clock.now();
    elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                  frameFinish - frameStart)
                  .count() *
              speed;
    // } while (lockFramerate && elapsed < frameDelta2);
    // if (1.0 / elapsed < 50) {
    //     printf("%.2f\n", 1.0 / elapsed);
    // }
    accumulator += elapsed;
    ctx.elapsed = ctx.frameCount = 0;
    totalElapsed += elapsed / 1000000.0;

    TaskManager::work(taskManager->mainQueue, *update);

    input.startFrame();
    handleEvents();
    input.endFrame();

    if (!running) {
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
        fixedUpdate(ctx);
    }
    if (accumulator > frameDelta2) {
        accumulator = fmod(accumulator, frameDelta2);
    }

    ctx.elapsed = elapsed / 1000000.0;
    this->update(ctx);
    if (!running) {
        quit();
        return false;
    }
    frameStart = frameFinish;

    phase = FramePhase::Render;
    TaskManager::work(taskManager->renderQueue, ctx);
    render(ctx);
    renderThread();

    phase = FramePhase::Inactive;

    fonts.flush();

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

void Engine::handleEvents() {
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
                        input.registerMouseOver(true);
                        break;
                    }
                    case SDL_WINDOWEVENT_LEAVE: {
                        input.registerMouseOver(false);
                        break;
                    }
                    case SDL_WINDOWEVENT_CLOSE: {
                        quit();
                        break;
                    }
                }
                break;
            }
            case SDL_KEYDOWN: {
                if (!event.key.repeat) {
                    if (handleKey) {
                        input.keyDown(
                            static_cast<Key>(event.key.keysym.scancode));
                    }
                }
                break;
            }
            case SDL_KEYUP: {
                if (handleKey) {
                    input.keyUp(static_cast<Key>(event.key.keysym.scancode));
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if (handleMouse) {
                    input.mouseDown(sdlMouseButton(event.button.button));
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if (handleMouse) {
                    input.mouseUp(sdlMouseButton(event.button.button));
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
                    input.mouseWheel(
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
        input.registerMousePos(mouseX, mouseY);
    }
}

void Engine::fixedUpdate(UpdateContext &ctx) {
#if TRACY_ENABLE
    ZoneScopedN("Engine::fixedUpdate");
#endif
    if (this->paused) {
        return;
    }
    invoke(onFixedUpdate, &ctx);
}

void Engine::update(UpdateContext &ctx) {
#if TRACY_ENABLE
    ZoneScopedN("Engine::update");
#endif
    if (this->paused) {
        return;
    }

    audio.update();
    // refresh window size
    int height = window.y();
    window.size();

    if (!cursor.empty() && (height != window.y() || !_cursor)) {
        chooseCursor();
    }

    // handle setTimeout events
    static std::list<TimedEvent> requeue;
    if (!this->events.empty()) {
        float elapsed = ctx.elapsed;
        elapsed -= this->events.front().delay;
        this->events.front().delay -= ctx.elapsed;
        while (!this->events.empty() && this->events.front().delay < 0) {
            TimedEvent &event = this->events.front();
            if (invoke(event.signal, false, &ctx, event.userData)) {
                requeue.emplace_back(event.interval, event.interval,
                                     event.signal, event.userData);
            }
            this->events.pop_front();
            if (!this->events.empty() && elapsed > 0) {
                float oldElapsed = elapsed;
                elapsed -= this->events.front().delay;
                this->events.front().delay -= oldElapsed;
            }
        }
        // re-enqueue events which returned true
        while (!requeue.empty()) {
            TimedEvent &event = requeue.front();
            this->setTimeout(event.signal, event.delay, event.userData);
            requeue.pop_front();
        }
    }

    // asset requests
    assets.update();

    // actual update cycle
    invoke(onUpdate, &ctx);
    script.update();
    invoke(postUpdate, &ctx);
}

void Engine::render(RenderContext &ctx) {
#if TRACY_ENABLE
    ZoneScopedN("Engine::render");
#endif
    checkForGlErrors("engine render");

    if (engine->window.skipFrames > 0) {
        --engine->window.skipFrames;
    } else {
        renderer.startFrame(ctx);
        invoke(onRender, &ctx);

        for (auto &camera : cameras) {
            ctx.drawCommandBuffer->buf.emplace_back<SetCamera>(&camera);
            ctx.camera = &camera;
            camera.update(ctx);
            ctx.drawCommandBuffer->setCamera(&camera);
            invoke(camera.render, &ctx);
        }
        ctx.camera = nullptr;

        fonts.commit();
        checkForGlErrors("fonts commit");

        renderer.renderFrame(ctx);
        checkForGlErrors("after render frame");

        invoke(this->postRender, &ctx);
    }
}

void Engine::renderThread() {
    TaskManager::work(taskManager->renderQueue, ctx);
    flip();
}

void Engine::flip() {
#if TRACY_ENABLE
    ZoneScopedN("Engine::flip");
#endif
    renderer.flip();
    checkForGlErrors("flush fonts");
}

void Engine::setTimeout(CustomSignal s, float delay, void *userData) {
    bool inserted = false;
    float interval = delay;
    for (auto it = this->events.begin(); it != this->events.end(); ++it) {
        if (it->delay <= 0)
            continue;
        if (delay < it->delay) {
            it->delay -= delay;
            this->events.emplace(it, delay, interval, s, userData);
            inserted = true;
            break;
        } else {
            delay -= it->delay;
        }
    }
    if (!inserted) {
        this->events.emplace_back(delay, interval, s, userData);
    }
}

void Engine::addCursor(const std::string &cursorPath,
                       const std::string &cursorName, int resolution) {
    std::string s = engine->io->readFile(cursorPath);

    TaskManager::instance->push([=, s = std::move(s)](UpdateContext &) mutable {
        SDL_RWops *rw = SDL_RWFromConstMem(s.c_str(), s.size());
        SDL_Surface *surface = IMG_LoadTyped_RW(rw, 0, "PNG");
        SDL_RWclose(rw);

        SDL_Cursor *cursor = SDL_CreateColorCursor(surface, 0, 0);

        TaskManager::instance->pushRender([=](RenderContext &) {
            this->cursors[cursorName].push_back(
                std::make_pair(resolution, cursor));
        });
    });
}

void Engine::setCursor(const std::string &cursor) {
    if (this->cursor != cursor) {
        this->cursor = cursor;
        chooseCursor();
    }
}

void Engine::chooseCursor() {
    auto &list = cursors[cursor];
    SDL_Cursor *candidate = nullptr;
    int candidateY = -1;
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i].second &&
            (list[i].first <= window.y() && list[i].first > candidateY)) {
            candidate = list[i].second;
            candidateY = list[i].first;
        }
    }
    if (candidate && candidate != _cursor) {
        SDL_SetCursor(_cursor = candidate);
    }
}

}
