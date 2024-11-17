#ifndef KRIT_INPUT_MOUSE
#define KRIT_INPUT_MOUSE

#include "krit/Math.h"
#include <vector>

namespace krit {

typedef int Action;
struct InputContext;

enum MouseButton {
    MouseLeft = 1,
    MouseMiddle,
    MouseRight,
    MouseWheel,
    MouseButtonMax,
};

struct MouseContext {
    Vec3i mousePos;
    bool mouseOver = false;

    std::array<Action, MouseButtonMax> mappings{0};
    std::array<bool, MouseButtonMax> active{0};

    MouseContext() : mousePos(-1, -1) {}

    void define(MouseButton btn, Action action) {
        this->mappings[btn] = action;
    }
    void undefine(MouseButton btn) { this->mappings[btn] = 0; }
    void registerMouseState(InputContext *ctx, MouseButton btn, int state);
    void registerOver(bool over) { this->mouseOver = over; }
    void registerPos(int x, int y) { this->mousePos.setTo(x, y); }
};

}

#endif
