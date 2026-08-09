#include "krit/Engine.h"
#if KRIT_ENABLE_TOOLS
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "krit/editor/Editor.h"
#endif
#include "krit/CrashHandler.h"
#include "krit/Options.h"
#include "krit/TaskManager.h"
#include "krit/input/InputContext.h"
#include "krit/input/Key.h"
#include "krit/input/Mouse.h"
#include "krit/render/Gl.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Panic.h"
#include "krit/utils/Profiling.h"
#include "krit/utils/Signal.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <stdlib.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#if TRACY_ENABLE
#include "Tracy.hpp"
#endif

namespace krit {

std::vector<Engine *> active;

Engine *engine{0};
RenderContext &render() {
    assert(engine);
    return engine->renderCtx();
}

#ifdef __EMSCRIPTEN__
static void __doFrame() {
    if (engine->running) {
        engine->runFrame();
    }
}
#endif

Engine::Engine(KritOptions &options)
    : platform(krit::platform()),
#if KRIT_ENABLE_NET
      net(krit::net()),
#endif
      window(options), renderer(window, options.block),
#if KRIT_ENABLE_SCRIPT
      script(this),
#endif
      fixedFramerate(options.fixedFramerate), block(options.block),
      taskManager(std::make_unique<TaskManager>(3)) {
    Scope _scope = scope();
    active.push_back(this);
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(__doFrame, 0, 0);
#endif
#if KRIT_ENABLE_SCRIPT
    script.userData = this;
    scriptContext = JS_NewObject(script.ctx);
#endif
    taskManager->start();
}

Engine::~Engine() {
    for (size_t i = 0; i < active.size(); ++i) {
        if (active[i] == this) {
            if (i == active.size() - 1) {
                active.pop_back();
            } else {
                active[i] = active[active.size() - 1];
                active.pop_back();
            }
            break;
        }
    }
#if KRIT_ENABLE_SCRIPT
    JS_FreeValue(script.ctx, scriptContext);
#endif
#if KRIT_ENABLE_CURSORS
    for (auto it : cursors) {
        for (auto &cursor : it.second) {
            SDL_DestroyCursor(cursor.second);
        }
    }
#endif
}

Engine::Scope::Scope(Engine *engine) : prev(krit::engine) {
    krit::engine = engine;
}

Engine::Scope::~Scope() { krit::engine = prev; }

float Engine::time() {
    return std::chrono::duration_cast<std::chrono::microseconds>(clock.now() -
                                                                 appStart)
               .count() /
           1000.0f;
}

void Engine::start() {
    Scope _scope = scope();

    CrashHandler::init();

    appStart = clock.now();

    frameDelta = 1000000 / fixedFramerate;
    frameDelta2 = 1000000 / (fixedFramerate + 2);

    ctx.camera = nullptr;
    ctx.drawCommandBuffer = &renderer.drawCommandBuffer;

    frameStart = clock.now();
    frameFinish = frameStart;
    // bool lockFramerate = true;

    phase = FramePhase::Begin;
    invoke(onBegin);
    phase = FramePhase::Inactive;
    onBegin = nullptr;

    running = true;
}

void Engine::run() {
    Scope _scope = scope();
    while (running) {
        if (!runFrame()) {
            break;
        }
    }

    cleanup();
}

void Engine::cleanup() {
    Scope _scope = scope();

    invoke(onEnd);
    onEnd = nullptr;
    engine->taskManager->cleanup();
}

bool Engine::runFrame() {
    Scope _scope = scope();

    if (!running) {
        return false;
    }

    ++frame.tickId;
    LOG_DEBUG("starting tick %u", frame.tickId);

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
    frame.elapsed = frame.frameCount = 0;
    totalElapsed += elapsed / 1000000.0;

    LOG_DEBUG("handling work queue");
    TaskManager::work(taskManager->mainQueue);

    LOG_DEBUG("handling input");
    handleEvents();
    input.endFrame();

    if (!running) {
        quit();
        return false;
    }

    if (block) {
        while (accumulator >= frameDelta2 && frame.frameCount < MAX_FRAMES) {
            accumulator -= frameDelta;
            if (accumulator < 0) {
                accumulator = 0;
            }
            ++frame.frameCount;
            ++frame.frameId;
            LOG_DEBUG("fixed update frame %u", frame.frameId);
            frame.elapsed = frameDelta / 1000000.0;
            fixedUpdate();
        }
        if (accumulator > frameDelta2) {
            accumulator = fmod(accumulator, frameDelta2);
        }
    }

    frame.elapsed = elapsed / 1000000.0;
    LOG_DEBUG("update; elapsed: %.6fms", frame.elapsed * 1000);
    this->update();
    if (!running) {
        quit();
        return false;
    }
    frameStart = frameFinish;

    LOG_DEBUG("render");
    phase = FramePhase::Render;
    TaskManager::work(taskManager->renderQueue);
    render();
    LOG_DEBUG("handling render work queue");
    TaskManager::work(taskManager->renderQueue);
    LOG_DEBUG("committing render");
    renderer.commit();

    phase = FramePhase::Inactive;

#if KRIT_ENABLE_TEXT
    LOG_DEBUG("flush fonts");
    fonts.flush();
#endif

#if TRACY_ENABLE
    FrameMark;
#endif

    // if there are multiple engines running, we may receive events before the
    // next frame, so clear our input state here
    input.startFrame();

    LOG_DEBUG("tick finished");
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
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        auto window = SDL_GetWindowFromEvent(&event);
        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            printf("WHEEL %p\n", window);
        }
        if (window) {
            for (auto &engine : active) {
                if (engine->window.window == window) {
                    engine->handleEvent(event);
                    break;
                }
            }
        }
    }
}

void Engine::handleEvent(SDL_Event &event) {
    auto scope = this->scope();
    bool handleKey = true, handleMouse = true;
#if KRIT_ENABLE_TOOLS
    if (Editor::imguiInitialized) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        auto &io = ImGui::GetIO();
        handleKey = !io.WantTextInput;
        handleMouse = !io.WantCaptureMouse;
    }
#endif
    switch (event.type) {
        case SDL_EVENT_QUIT: {
            quit();
            break;
        }
        case SDL_EVENT_WINDOW_MOUSE_ENTER: {
            input.registerMouseOver(true);
            break;
        }
        case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
            input.registerMouseOver(false);
            break;
        }
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
            quit();
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            if (!event.key.repeat) {
                if (handleKey) {
                    input.keyDown(static_cast<KeyCode>(event.key.scancode));
                }
            }
            break;
        }
        case SDL_EVENT_KEY_UP: {
            if (handleKey) {
                input.keyUp(static_cast<KeyCode>(event.key.scancode));
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (handleMouse) {
                input.mouseDown(sdlMouseButton(event.button.button));
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            if (handleMouse) {
                input.mouseUp(sdlMouseButton(event.button.button));
            }
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            puts("wheel event");
            if (handleMouse && event.wheel.y) {
                printf("y: %.2f\n", (double)event.wheel.y);
                input.mouseWheel(
                    event.wheel.y *
                    (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? 1 : -1));
            }
            break;
        }
        case SDL_EVENT_TEXT_INPUT: {
            input.key.inputText += event.text.text;
            break;
        }
    }
}

void Engine::fixedUpdate() {
    ProfileZone("Engine::fixedUpdate");
    if (this->paused) {
        return;
    }
    invoke(onFixedUpdate);
}

void Engine::update() {
    ProfileZone("Engine::update");
    if (this->paused) {
        return;
    }

#if (KRIT_ENABLE_SOUND && !KRIT_SOUND_THREAD)
    audio.update();
#endif
    // refresh window size
    window.size();

#if KRIT_ENABLE_CURSORS
    int height = window.y;
    if (!cursor.empty() && (height != window.y || !_cursor)) {
        chooseCursor();
    }
#endif

    // handle setTimeout events
    static std::list<TimedEvent> requeue;
    if (!this->events.empty()) {
        float elapsed = frame.elapsed;
        elapsed -= this->events.front().delay;
        this->events.front().delay -= frame.elapsed;
        while (!this->events.empty() && this->events.front().delay < 0) {
            TimedEvent &event = this->events.front();
            if (invoke(event.signal, false, event.userData)) {
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
    invoke(onUpdate);
#if KRIT_ENABLE_SCRIPT
    script.update();
#endif
    invoke(postUpdate);
}

void Engine::render() {
    ProfileZone("Engine::render");
    checkForGlErrors("engine render");

    if (engine->window.skipFrames > 0) {
        --engine->window.skipFrames;
    } else {
        invoke(onRender);

        for (auto &camera : cameras) {
            ctx.drawCommandBuffer->buf.emplace_back<SetCamera>(&camera);
            ctx.camera = &camera;
            camera.update();
            ctx.drawCommandBuffer->setCamera(&camera);
            invoke(camera.render);
        }
        ctx.camera = nullptr;

#if KRIT_ENABLE_TEXT
        fonts.commit();
        checkForGlErrors("fonts commit");
#endif

        renderer.renderFrame(ctx);
        checkForGlErrors("after render frame");

        invoke(postRender);
    }
}

void Engine::setTimeout(TimedEvent::SignalType s, float delay, void *userData) {
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

#if KRIT_ENABLE_CURSORS
void Engine::addCursor(const std::string &cursorPath,
                       const std::string &cursorName, int resolution, int x,
                       int y) {
    std::string s = engine->io->readFile(cursorPath);

    SDL_IOStream *rw = SDL_IOFromConstMem(s.c_str(), s.size());
    SDL_Surface *surface = IMG_LoadTyped_IO(rw, 0, "PNG");
    SDL_CloseIO(rw);

    SDL_Cursor *cursor = SDL_CreateColorCursor(surface, x, y);
    this->cursors[cursorName].push_back(std::make_pair(resolution, cursor));
    if (this->cursor == cursorName) {
        chooseCursor();
    }

    SDL_DestroySurface(surface);
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
            (list[i].first <= window.y && list[i].first > candidateY)) {
            candidate = list[i].second;
            candidateY = list[i].first;
        }
    }
    if (candidate && candidate != _cursor) {
        SDL_SetCursor(_cursor = candidate);
    }
}
#endif

Camera &Engine::addCamera() {
    cameras.emplace_back();
    return cameras.back();
}

}
