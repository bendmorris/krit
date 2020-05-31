#ifndef KRIT_INPUT_MOUSE_CONTEXT
#define KRIT_INPUT_MOUSE_CONTEXT

#include "krit/utils/Signal.h"
#include "krit/Sprite.h"
#include <list>
#include <utility>
#include <vector>

namespace krit {

struct MouseCallbacks {
    SelfUpdateSignal onPress = nullptr;
    SelfUpdateSignal onRelease = nullptr;
    SelfUpdateSignal onEnter = nullptr;
    SelfUpdateSignal onExit = nullptr;
    bool fallthrough = false;

    MouseCallbacks(): MouseCallbacks(nullptr, nullptr, nullptr, nullptr) {}
    MouseCallbacks(SelfUpdateSignal onPress, SelfUpdateSignal onRelease, SelfUpdateSignal onEnter, SelfUpdateSignal onExit, bool fallthrough = false)
     : onPress(onPress), onRelease(onRelease), onEnter(onEnter), onExit(onExit), fallthrough(fallthrough) {}
};

struct MouseContext: public Sprite {
    MouseCallbacks defaultCallbacks;
    bool active = true;

    MouseContext() {}
    MouseContext(MouseCallbacks defaults): defaultCallbacks(defaults) {}

    /**
     * Adds an object to the MouseContext registry.
     *
     * @param    sprite
     * @param    onPress        Callback when mouse is pressed down over this object.
     * @param    onRelease      Callback when mouse is released over this object.
     * @param    onEnter        Callback when mouse is this object.
     * @param    onExit         Callback when mouse moves out of this object.
     * @param    fallThrough    If true, other objects overlaped by this will still receive mouse events.
     */
    void add(VisibleSprite *sprite, MouseCallbacks callbacks) {
        this->registeredObjects.emplace_back(sprite, callbacks);
    }

    void remove(VisibleSprite *sprite) {
        for (auto it = this->registeredObjects.begin(); it != this->registeredObjects.end(); ++it) {
            if (it->first == sprite) {
                this->registeredObjects.erase(it);
                break;
            }
        }
    }

    void clear() {
        this->registeredObjects.clear();
        this->collisions.clear();
        this->lastCollisions.clear();
    }

    void update(UpdateContext &ctx) override;

    private:
        std::list<std::pair<VisibleSprite*, MouseCallbacks>> registeredObjects;
        std::list<std::pair<VisibleSprite*, MouseCallbacks>> collisions;
        std::list<std::pair<VisibleSprite*, MouseCallbacks>> lastCollisions;
        bool lastFallThrough = false;
};

}

#endif
