#include "krit/input/InputContext.h"

namespace krit {

void KeyContext::registerKeyState(InputContext *ctx, Key keyCode, int state) {
    auto it = keyMappings.find(keyCode);
    if (it != keyMappings.end()) {
        ctx->addEvent(it->second, state);
    }
}

void MouseContext::registerMouseState(InputContext *ctx, MouseButton btn,
                                      int state) {
    Action it = mappings[btn];
    if (it) {
        ctx->addEvent(it, state);
    }
}

}
