#include "krit/Engine.h"

#include "krit/render/DrawCommand.h"
#include "krit/UpdateContext.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Matrix.h"
#include "krit/math/Rectangle.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"

namespace krit {

void Engine::fixedUpdate(UpdateContext &ctx) {
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
    if (this->paused) {
        return;
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
                requeue.emplace_back(event.interval, event.interval, event.signal, event.userData);
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

    // actual update cycle
    invoke(onUpdate, &ctx);
    camera.update(ctx);
    uiCamera.update(ctx);
    for (auto &tree : trees) {
        if (tree.root) {
            ctx.camera = tree.camera;
            tree.root->update(ctx);
        }
    }
    invoke(postUpdate, &ctx);
}

void Engine::render(RenderContext &ctx) {
    ctx.userData = userData;

    invoke(onRender, &ctx);

    if (this->bgColor.a > 0) {
        DrawKey key;
        IntRectangle windowRect(0, 0, ctx.window->width(), ctx.window->height());
        Matrix m;
        ctx.drawCommandBuffer->addRect(
            ctx,
            key,
            windowRect,
            m,
            this->bgColor
        );
    }
    
    for (auto &tree : trees) {
        if (tree.root) {
            ctx.camera = tree.camera;
            tree.root->render(ctx);
        }
    }

    invoke(this->postRender, &ctx);
}

void Engine::setTimeout(CustomSignal s, float delay, void *userData) {
    bool inserted = false;
    float interval = delay;
    for (auto it = this->events.begin(); it != this->events.end(); ++it) {
        if (it->delay <= 0) continue;
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

}