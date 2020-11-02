#include "krit/input/FocusContext.h"

namespace krit {

void FocusContext::setFocus(UpdateContext *ctx, VisibleSprite *sprite) {
    auto lostFocus = this->focus;
    for (size_t i = 0; i < registeredObjects.size(); ++i) {
        if (registeredObjects[i].first == sprite) {
            focus = i;
            break;
        }
    }
    auto gotFocus = this->focus;
    if (lostFocus != gotFocus) {
        if (registeredObjects[lostFocus].first) {
            invoke(registeredObjects[lostFocus].second.onLostFocus, ctx, static_cast<void*>(registeredObjects[lostFocus].first));
        }
        if (registeredObjects[gotFocus].first) {
            invoke(registeredObjects[gotFocus].second.onGotFocus, ctx, static_cast<void*>(registeredObjects[gotFocus].first));
        }
    }
}

void FocusContext::changeFocus(UpdateContext *ctx, bool forward) {
    auto lostFocus = this->focus;
    this->focus += (forward ? 1 : -1);
    if (this->focus < 0) {
        this->focus = this->registeredObjects.size() - 1;
    }
    if (this->focus >= static_cast<int>(this->registeredObjects.size())) {
        this->focus = 0;
    }
    auto gotFocus = this->focus;
    if (lostFocus != gotFocus) {
        if (registeredObjects[lostFocus].first) {
            invoke(registeredObjects[lostFocus].second.onLostFocus, ctx, static_cast<void*>(registeredObjects[lostFocus].first));
        }
        if (registeredObjects[gotFocus].first) {
            invoke(registeredObjects[gotFocus].second.onGotFocus, ctx, static_cast<void*>(registeredObjects[gotFocus].first));
        }
    }
}

}
