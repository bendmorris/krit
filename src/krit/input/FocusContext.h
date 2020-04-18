#ifndef KRIT_INPUT_FOCUSMANAGER
#define KRIT_INPUT_FOCUSMANAGER

#include "krit/utils/Signal.h"
#include "krit/Sprite.h"
#include <list>
#include <string>
#include <utility>

using namespace std;
using namespace krit;

namespace krit {

struct FocusCallbacks {
    SelfUpdateSignal onGotFocus = nullptr;
    SelfUpdateSignal onLostFocus = nullptr;

    FocusCallbacks(): FocusCallbacks(nullptr, nullptr) {}
    FocusCallbacks(SelfUpdateSignal onGotFocus, SelfUpdateSignal onLostFocus)
     : onGotFocus(onGotFocus), onLostFocus(onLostFocus) {}
};

class FocusContext {
    public:
        bool loop = true;

        void add(UpdateContext *ctx, VisibleSprite *sprite, SelfUpdateSignal onGotFocus = nullptr, SelfUpdateSignal onLostFocus = nullptr) {
            bool wasEmpty = this->registeredObjects.empty();
            this->registeredObjects.push_back(make_pair(sprite, FocusCallbacks(onGotFocus, onLostFocus)));
            if (wasEmpty) {
                this->focus = this->registeredObjects.begin();
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

        void clear() {
            this->registeredObjects.clear();
        }

        VisibleSprite *focused() {
            return this->registeredObjects.empty() ? nullptr : this->focus->first;
        }

        void setFocus(UpdateContext *ctx, VisibleSprite *sprite);
        void prevFocus(UpdateContext *ctx) { this->changeFocus(ctx, false); }
        void nextFocus(UpdateContext *ctx) { this->changeFocus(ctx, true); }

    private:
        list<pair<VisibleSprite*, FocusCallbacks>> registeredObjects;
        list<pair<VisibleSprite*, FocusCallbacks>>::iterator focus;

        void changeFocus(UpdateContext *ctx, bool forward);
};

}

#endif
