#ifndef KRIT_ENGINE
#define KRIT_ENGINE

#include "krit/asset/AssetContext.h"
#include "krit/input/ControlBindings.h"
#include "krit/input/InputContext.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Color.h"
#include "krit/utils/Signal.h"
#include "krit/Camera.h"
#include "krit/Sprite.h"
#include <list>

using namespace std;
using namespace krit;

namespace krit {

struct TimedEvent {
    double delay;
    double interval;
    CustomSignal signal;
    void *userData;

    TimedEvent(double delay, double interval, CustomSignal signal, void *userData)
        : delay(delay), interval(interval), signal(signal), userData(userData) {}
};

struct Engine {
    bool paused = false;
    bool fixedFrameRate = false;
    bool finished = false;

    UpdateSignal onBegin = nullptr;
    UpdateSignal onUpdate = nullptr;
    UpdateSignal postUpdate = nullptr;
    RenderSignal onRender = nullptr;
    RenderSignal postRender = nullptr;
    list<TimedEvent> events;

    // global context
    ControlBindings controls;
    InputContext input;
    AssetCache assetCache;
    AssetContext asset;

    Color bgColor = Color::black();
    Camera camera;
    Camera uiCamera;

    unique_ptr<Sprite> root = nullptr;
    void *userData = nullptr;

    Engine(): asset(&this->assetCache), input(&this->controls) {}

    void reset() {
        this->controls.reset();
    }

    void quit() {
        this->finished = true;
    }

    void update(UpdateContext &ctx) {
        if (this->paused) {
            return;
        }
        ctx.asset = &this->asset;
        ctx.camera = &this->camera;
        ctx.controls = &this->controls;
        ctx.input = &this->input;
        ctx.userData = this->userData;

        // handle setTimeout events
        static std::list<TimedEvent> requeue;
        if (!this->events.empty()) {
            double elapsed = ctx.elapsed;
            elapsed -= this->events.front().delay;
            this->events.front().delay -= ctx.elapsed;
            while (!this->events.empty() && this->events.front().delay < 0) {
                TimedEvent &event = this->events.front();
                if (invoke(event.signal, false, &ctx, event.userData)) {
                    requeue.emplace_back(event.interval, event.interval, event.signal, event.userData);
                }
                this->events.pop_front();
                if (!this->events.empty() && elapsed > 0) {
                    double oldElapsed = elapsed;
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
        invoke(this->onUpdate, &ctx);
        this->camera.update(ctx);
        this->uiCamera.update(ctx);
        this->controls.update(ctx);
        this->input.update(ctx);
        if (this->root) this->root->update(ctx);
        invoke(this->postUpdate, &ctx);
    }

    void render(RenderContext &ctx) {
        ctx.camera = &this->camera;
        invoke(this->onRender, &ctx);

        if (this->bgColor.a > 0) {
            DrawKey key;
            IntRectangle windowRect(0, 0, ctx.window->width(), ctx.window->height());
            Matrix m;
            ctx.drawCommandBuffer->addRect(
                key,
                windowRect,
                m,
                this->bgColor
            );
        }
        if (this->root) this->root->render(ctx);

        invoke(this->postRender, &ctx);
    }

    void setTimeout(CustomSignal s, double delay = 0, void *userData = nullptr) {
        bool inserted = false;
        double interval = delay;
        for (auto it = this->events.begin(); it != this->events.end(); ++it) {
            if (it->delay <= 0) continue;
            if (delay < it->delay) {
                it->delay -= delay;
                this->events.emplace(it, delay, interval, s, userData);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            this->events.emplace_back(delay, interval, s, userData);
        }
    }

    void setRoot(Sprite *root) {
        this->root = unique_ptr<Sprite>(root);
    }
};

}

#endif
