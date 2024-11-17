#include "krit/input/Mouse.h"
#include "krit/input/InputContext.h"

namespace krit {

void MouseContext::registerMouseState(InputContext *ctx, MouseButton btn,
                                      int state) {
    Action action = mappings[btn];
    if (action) {
        ctx->addEvent(action, state);
    }
}

}
