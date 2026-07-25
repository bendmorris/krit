#include "krit/input/Key.h"
#include "krit/Engine.h"
#include "krit/input/InputContext.h"
#include <cassert>

namespace krit {

void KeyContext::registerKeyState(InputContext *ctx, KeyCode keyCode,
                                  int state) {
    int idx = static_cast<int>(keyCode);
    if (idx >= 512) {
        return;
    }
    auto action = keyMappings[idx];
    if (action) {
        ctx->addEvent(action, state);
    }
}

void KeyContext::startTextInput() { SDL_StartTextInput(engine->window.window); }

void KeyContext::stopTextInput() { SDL_StopTextInput(engine->window.window); }

void KeyContext::define(KeyCode keyCode, Action action) {
    int idx = static_cast<int>(keyCode);
    assert(idx >= 0);
    assert(idx < 0x200);
    this->keyMappings[idx] = action;
}
void KeyContext::undefine(KeyCode keyCode) {
    int idx = static_cast<int>(keyCode);
    assert(idx >= 0);
    assert(idx < 0x200);
    this->keyMappings[idx] = 0;
}

const std::string &KeyContext::keyName(KeyCode keyCode) {
    int idx = static_cast<int>(keyCode);
    assert(idx >= 0);
    assert(idx < 0x200);
    auto &it = keyNames[idx];
    if (it.empty()) {
        keyNames[idx] = SDL_GetScancodeName((SDL_Scancode)keyCode);
    }
    return it;
}

}
