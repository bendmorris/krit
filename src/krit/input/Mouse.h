#ifndef KRIT_INPUT_MOUSE
#define KRIT_INPUT_MOUSE

#include "krit/Math.h"
#include <vector>

namespace krit {

struct InputContext;
struct UpdateContext;

enum MouseButton {
    MouseLeft = 1,
    MouseMiddle,
    MouseRight,
    MouseWheel,
    MouseButtonMax,
};

}

#endif
