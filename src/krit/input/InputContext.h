#ifndef KRIT_INPUT_INPUT
#define KRIT_INPUT_INPUT

#include "krit/utils/Signal.h"
#include "krit/input/Key.h"
#include "krit/input/Mouse.h"
#include "krit/UpdateContext.h"
#include <string>
#include <unordered_map>
#include <utility>

namespace krit {

typedef int Action;

struct ActionEvent {
    Action action;
    int state;
    int prevState;
};

struct InputContext {
    struct KeyManager {
        std::unordered_map<int, Action> keyMappings;

        KeyManager() {}

        void define(Key keyCode, Action action) {
            this->keyMappings.insert(std::make_pair(keyCode, action));
        }

        void undefine(Key keyCode) {
            this->keyMappings.erase(keyCode);
        }

        void registerKeyState(InputContext &ctx, Key keyCode, int state) {
            auto it = keyMappings.find(keyCode);
            if (it != keyMappings.end()) {
                ctx.addEvent(it->second, state);
            }
        }
    };

    struct MouseManager {
        Point mousePos;
        bool mouseOver = false;

        Action mappings[4] = {0};
        bool active[4] = {0};

        MouseManager(): mousePos(-1, -1) {}

        void define(MouseButton btn, Action action) {
            this->mappings[btn] = action;
        }

        void undefine(MouseButton btn) {
            this->mappings[btn] = 0;
        }

        void registerMouseState(InputContext &ctx, MouseButton btn, int state) {
            Action it = mappings[btn];
            if (it) {
                ctx.addEvent(it, state);
            }
        }

        void registerOver(bool over) {
            this->mouseOver = over;
        }

        void registerPos(int x, int y) {
            this->mousePos.setTo(x, y);
        }
    };

    KeyManager key;
    MouseManager mouse;

    std::unordered_map<Action, int> states;
    std::unordered_map<Action, bool> seen;
    std::vector<ActionEvent> events;

    InputContext() {}

    int state(Action action) { return states[action]; }

    void startFrame() {
        seen.clear();
        events.clear();
    }

    void addEvent(Action action, int state) {
        events.emplace_back((ActionEvent) { .action = action, .state = state, .prevState = states[action] });
        states[action] = state;
        seen[action] = true;
    }

    void endFrame() {
        auto it = states.begin();
        while (it != states.end()) {
            if (!seen[it->first]) {
                if (it->second) {
                    addEvent(it->first, it->second);
                    ++it;
                } else {
                    it = states.erase(it);
                }
            } else {
                ++it;
            }
        }
    }

    void keyDown(Key key) { this->key.registerKeyState(*this, key, 1); }
    void keyUp(Key key) { this->key.registerKeyState(*this, key, 0); }
    void mouseDown(MouseButton btn) { this->mouse.registerMouseState(*this, btn, 1); }
    void mouseUp(MouseButton btn) { this->mouse.registerMouseState(*this, btn, 0); }

    void bindKey(Key key, Action action) { this->key.define(key, action); }
    void bindMouse(MouseButton btn, Action action) { this->mouse.define(btn, action); }
    void registerMousePos(int x, int y) { mouse.registerPos(x, y); }
    void registerMouseOver(bool over) { mouse.registerOver(over); }
};

}
#endif
