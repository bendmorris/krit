#ifndef KRIT_INPUT_FOCUSMANAGER
#define KRIT_INPUT_FOCUSMANAGER

#include "krit/utils/Signal.h"
#include "krit/Sprite.h"
#include <list>
#include <string>
#include <utility>

namespace krit {

struct FocusCallbacks {
    SelfUpdateSignal onGotFocus = nullptr;
    SelfUpdateSignal onLostFocus = nullptr;

    FocusCallbacks(): FocusCallbacks(nullptr, nullptr) {}
    FocusCallbacks(SelfUpdateSignal onGotFocus, SelfUpdateSignal onLostFocus)
     : onGotFocus(onGotFocus), onLostFocus(onLostFocus) {}
};

struct FocusContext {
    bool loop = true;

    void add(UpdateContext *ctx, VisibleSprite *sprite, SelfUpdateSignal onGotFocus = nullptr, SelfUpdateSignal onLostFocus = nullptr) {
        bool wasEmpty = this->registeredObjects.empty();
        this->registeredObjects.emplace_back(sprite, FocusCallbacks(onGotFocus, onLostFocus));
        if (wasEmpty) {
            this->focus = 0;
            invoke(onGotFocus, ctx, static_cast<void*>(sprite));
        }
    }

    void remove(VisibleSprite *sprite) {
        for (auto it = this->registeredObjects.begin(); it != this->registeredObjects.end(); ++it) {
            if (it->first == sprite) {
                this->registeredObjects.erase(it);
                break;
            }
        }
    }

    VisibleSprite *get(size_t index) { return registeredObjects[focus].first; }

    void clear() {
        this->registeredObjects.clear();
    }

    VisibleSprite *focused() {
        return this->registeredObjects.empty() ? nullptr : registeredObjects[focus].first;
    }

    void setFocus(UpdateContext *ctx, VisibleSprite *sprite);
    void prevFocus(UpdateContext *ctx) { this->changeFocus(ctx, false); }
    void nextFocus(UpdateContext *ctx) { this->changeFocus(ctx, true); }

    private:
        std::vector<std::pair<VisibleSprite*, FocusCallbacks>> registeredObjects;
        int focus = 0;

        void changeFocus(UpdateContext *ctx, bool forward);
};

}

#endif
