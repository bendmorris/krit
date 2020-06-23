#ifndef KRIT_INPUT_MOUSE
#define KRIT_INPUT_MOUSE

#include "krit/Math.h"
#include "krit/input/InputEvent.h"
#include <vector>

namespace krit {

class InputContext;
class UpdateContext;

enum MouseButton {
    MousePosition = 0,
    MouseLeft = 1,
    MouseMiddle = 2,
    MouseRight = 3,
};

struct MouseManager {
    Point mousePos;
    bool pressed = false;
    bool released = false;
    bool mouseOver = false;

    Action mappings[4] = {0};
    bool active[4] = {0};

    MouseManager(std::vector<InputEvent> *events): events(events) {}

    void reset() {
        this->pressed = this->released = false;
    }

    void update() {
        for (int i = 1; i < 4; ++i) {
            if (this->active[i]) {
                this->mouseEvent(static_cast<MouseButton>(i), InputActive);
            }
        }
    }

    void define(MouseButton btn, Action action) {
        this->mappings[btn] = action;
    }

    void undefine(MouseButton btn) {
        this->mappings[btn] = 0;
    }

    void registerDown(MouseButton btn) {
        if (btn == MouseLeft) {
            this->pressed = true;
        }
        this->active[btn] = true;
        if (this->mappings[btn] && !this->active[btn]) {
            this->mouseEvent(btn, InputStart);
        }
    }

    void registerUp(MouseButton btn) {
        if (btn == MouseLeft) {
            this->released = true;
        }
        this->active[btn] = false;
        if (btn == MouseLeft || (this->mappings[btn] && this->active[btn])) {
            this->mouseEvent(btn, InputFinish);
        }
    }

    void registerOver(bool over) {
        this->mouseOver = over;
    }

    void registerPos(int x, int y) {
        this->mousePos.setTo(x, y);
    }

    private:
        std::vector<InputEvent> *events;

        void mouseEvent(MouseButton btn, InputEventType eventType) {
            auto &found = this->mappings[btn];
            if (found) {
                this->events->emplace_back(eventType, found, LevelData(eventType != InputFinish));
            }
        }
};

}

#endif
