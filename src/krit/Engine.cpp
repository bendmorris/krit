#include "krit/Engine.h"

#include "krit/TaskManager.h"
#include "krit/UpdateContext.h"
#include "krit/io/Io.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Matrix.h"
#include "krit/math/Rectangle.h"
#include "krit/render/DrawCommand.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#if TRACY_ENABLE
#include "krit/tracy/Tracy.hpp"
#endif

namespace krit {

Engine::Engine(KritOptions &options) : window(options), renderer(window) {
    script.userData = this;
}

void Engine::fixedUpdate(UpdateContext &ctx) {
    #if TRACY_ENABLE
    ZoneScopedN("Engine::fixedUpdate");
    #endif
    if (this->paused) {
        return;
    }
    for (auto &tree : trees) {
        if (tree.root) {
            ctx.camera = tree.camera;
            tree.root->fixedUpdate(ctx);
        }
    }
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
    int height = window.height();
    window.size();

    if (!cursor.empty() && (height != window.height() || !_cursor)) {
        chooseCursor();
    }

    ctx.userData = this->userData;

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
    camera.update(ctx);
    uiCamera.update(ctx);
    script.update();
    for (auto &tree : trees) {
        if (tree.root) {
            ctx.camera = tree.camera;
            tree.root->update(ctx);
        }
    }
    camera.update(ctx);
    uiCamera.update(ctx);
    invoke(postUpdate, &ctx);
}

void Engine::render(RenderContext &ctx) {
    #if TRACY_ENABLE
    ZoneScopedN("Engine::render");
    #endif
    checkForGlErrors("engine render");

    ctx.userData = userData;

    fonts.commit();

    if (ctx.window->skipFrames > 0) {
        --ctx.window->skipFrames;
    } else {
        invoke(onRender, &ctx);

        for (auto &tree : trees) {
            if (tree.root) {
                ctx.camera = tree.camera;
                tree.root->render(ctx);
            }
        }

        invoke(this->postRender, &ctx);
    }

    checkForGlErrors("before render frame");
    renderer.renderFrame(ctx);
    checkForGlErrors("after render frame");
}

void Engine::flip(RenderContext &ctx) {
    #if TRACY_ENABLE
    ZoneScopedN("Engine::flip");
    #endif
    renderer.flip(ctx);
    fonts.flush();
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

void Engine::addTree(Sprite *root, Camera *camera) {
    if (!camera) {
        camera = &this->camera;
    }
    trees.emplace_back(root, camera);
}

void Engine::setRoot(int index, Sprite *root) {
    trees[index].root = std::unique_ptr<Sprite>(root);
}

void Engine::addCursor(const std::string &cursorPath,
                       const std::string &cursorName, int resolution) {
    int len;
    char *s = IoRead::read(cursorPath, &len);

    TaskManager::instance->push([=](UpdateContext &) {
        SDL_RWops *rw = SDL_RWFromConstMem(s, len);
        SDL_Surface *surface = IMG_LoadTyped_RW(rw, 0, "PNG");
        SDL_RWclose(rw);
        IoRead::free(s);

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
            (list[i].first <= window.height() && list[i].first > candidateY)) {
            candidate = list[i].second;
            candidateY = list[i].first;
        }
    }
    if (candidate && candidate != _cursor) {
        SDL_SetCursor(_cursor = candidate);
    }
}

}