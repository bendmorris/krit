#include "krit/input/FocusContext.h"

namespace krit {

void FocusContext::setFocus(UpdateContext *ctx, VisibleSprite *sprite) {
    auto lostFocus = this->focus;
    for (auto it = this->registeredObjects.begin(); it != this->registeredObjects.end(); ++it) {
        if (it->first == sprite) {
            this->focus = it;
            break;
        }
    }
    auto gotFocus = this->focus;
    if (lostFocus != gotFocus) {
        if (lostFocus->first) {
            invoke(lostFocus->second.onLostFocus, ctx, static_cast<void*>(lostFocus->first));
        }
        if (gotFocus->first) {
            invoke(gotFocus->second.onGotFocus, ctx, static_cast<void*>(gotFocus->first));
        }
    }
}

void FocusContext::changeFocus(UpdateContext *ctx, bool forward) {
    auto lostFocus = this->focus;
    if (forward) {
        if (++this->focus == this->registeredObjects.end()) {
            this->loop ? (this->focus = this->registeredObjects.begin()) : this->focus--;
        }
    } else {
        if (this->focus != this->registeredObjects.begin()) {
            this->focus--;
        } else {
            this->focus = this->registeredObjects.end();
            this->focus--;
        }
    }
    auto gotFocus = this->focus;
    if (lostFocus != gotFocus) {
        if (lostFocus->first) {
            invoke(lostFocus->second.onLostFocus, ctx, static_cast<void*>(lostFocus->first));
        }
        if (gotFocus->first) {
            invoke(gotFocus->second.onGotFocus, ctx, static_cast<void*>(gotFocus->first));
        }
    }
}

}
