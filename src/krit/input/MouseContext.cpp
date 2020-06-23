#include "krit/editor/Editor.h"
#include "krit/input/MouseContext.h"
#include "krit/input/Mouse.h"
#include "krit/Engine.h"
#include "krit/Math.h"

namespace krit {

void MouseContext::update(UpdateContext &ctx) {
    MouseManager &mouse = ctx.engine->controls.mouse;
    Point mousePos = mouse.mousePos;
    ctx.camera->untransformPoint(nullptr, mousePos);

    bool fellThrough = true;
    if (this->active) {
        for (auto &e : this->registeredObjects) {
            Rectangle r(e.first->getPosition(), e.first->getSize());
            // FIXME: scene coordinates?
            if (r.contains(mousePos)) {
                this->collisions.push_back(e);
                if (!e.second.fallthrough) {
                    fellThrough = false;
                    break;
                }
            }
        }

        for (auto &e : this->collisions) {
            if (e.second.onEnter) {
                bool existing = false;
                for (auto &it : this->lastCollisions) {
                    if (it.first == e.first) {
                        existing = true;
                        break;
                    }
                }
                if (!existing) {
                    ctx.engine->setTimeout([e, &ctx](UpdateContext*, void*) {
                        invoke(e.second.onEnter, &ctx, static_cast<void*>(e.first));
                        return false;
                    });
                }
            }
            if (mouse.pressed && e.second.onPress) {
                ctx.engine->setTimeout([e, &ctx](UpdateContext*, void*) {
                    invoke(e.second.onPress, &ctx, static_cast<void*>(e.first));
                    return false;
                });
            }
            if (mouse.released && e.second.onRelease) {
                ctx.engine->setTimeout([e, &ctx](UpdateContext*, void*) {
                    invoke(e.second.onRelease, &ctx, static_cast<void*>(e.first));
                    return false;
                });
            }
        }
        for (auto &e : this->lastCollisions) {
            if (e.second.onExit) {
                bool existing = false;
                for (auto &it : this->collisions) {
                    if (it.first == e.first) {
                        existing = true;
                        break;
                    }
                }
                if (!existing) {
                    invoke(e.second.onExit, &ctx, static_cast<void*>(e.first));
                }
            }
        }

        if (fellThrough && !this->lastFallThrough) {
            invoke(this->defaultCallbacks.onEnter, &ctx, static_cast<void*>(nullptr));
            // TODO: press, release
        } else if (this->lastFallThrough) {
            invoke(this->defaultCallbacks.onExit, &ctx, static_cast<void*>(nullptr));
        }
    }

    this->lastCollisions.clear();
    this->collisions.swap(this->lastCollisions);
    this->lastFallThrough = fellThrough;
}

}
