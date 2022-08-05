#ifndef KRIT_INPUT_INPUT
#define KRIT_INPUT_INPUT

#include "krit/UpdateContext.h"
#include "krit/input/Key.h"
#include "krit/input/Mouse.h"
#include "krit/utils/Signal.h"
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

struct InputContext;

struct KeyContext {
    std::unordered_map<int, Action> keyMappings;
    std::unordered_map<int, std::string> keyNames;

    KeyContext() {}

    void define(Key keyCode, Action action) {
        this->keyMappings.insert(std::make_pair(keyCode, action));
    }
    void undefine(Key keyCode) { this->keyMappings.erase(keyCode); }
    void registerKeyState(InputContext *ctx, Key keyCode, int state);

    const std::string &keyName(Key keyCode) {
        auto it = keyNames.find(keyCode);
        if (it == keyNames.end()) {
            auto x = keyNames.emplace(
                std::make_pair(keyCode, SDL_GetScancodeName((SDL_Scancode)keyCode)));
            return x.first->second;
        }
        return it->second;
    }
};

struct MouseContext {
    Vec3i mousePos;
    bool mouseOver = false;

    Action mappings[MouseButtonMax] = {0};
    bool active[MouseButtonMax] = {0};

    MouseContext() : mousePos(-1, -1) {}

    void define(MouseButton btn, Action action) {
        this->mappings[btn] = action;
    }
    void undefine(MouseButton btn) { this->mappings[btn] = 0; }
    void registerMouseState(InputContext *ctx, MouseButton btn, int state);
    void registerOver(bool over) { this->mouseOver = over; }
    void registerPos(int x, int y) { this->mousePos.setTo(x, y); }
};

struct InputContext {
    KeyContext key;
    MouseContext mouse;

    std::unordered_map<Action, int> states;
    std::unordered_map<Action, bool> seen;
    std::vector<ActionEvent> events;

    InputContext() {}

    int state(Action action) { return states[action]; }

    void startFrame() {
        seen.clear();
        events.clear();
    }

    void addEvent(Action action, int state, bool prepend = false) {
        if (prepend) {
            events.emplace(events.begin(),
                           (ActionEvent){.action = action,
                                         .state = state,
                                         .prevState = states[action]});
        } else {
            events.emplace_back((ActionEvent){
                .action = action, .state = state, .prevState = states[action]});
        }
        states[action] = state;
        seen[action] = true;
    }

    void endFrame() {
        auto it = states.begin();
        while (it != states.end()) {
            if (!seen[it->first]) {
                if (it->second) {
                    addEvent(it->first, it->second, true);
                    ++it;
                } else {
                    it = states.erase(it);
                }
            } else {
                ++it;
            }
        }
    }

    void keyDown(Key key) { this->key.registerKeyState(this, key, 1); }
    void keyUp(Key key) { this->key.registerKeyState(this, key, 0); }
    void mouseDown(MouseButton btn) {
        this->mouse.registerMouseState(this, btn, 1);
    }
    void mouseUp(MouseButton btn) {
        this->mouse.registerMouseState(this, btn, 0);
    }
    void mouseWheel(int y) {
        this->mouse.registerMouseState(this, MouseWheel, y);
        this->mouse.registerMouseState(this, MouseWheel, 0);
    }

    void bindKey(Key key, Action action) { this->key.define(key, action); }
    void bindMouse(MouseButton btn, Action action) {
        this->mouse.define(btn, action);
    }
    void registerMousePos(int x, int y) { mouse.registerPos(x, y); }
    void registerMouseOver(bool over) { mouse.registerOver(over); }
};

}
#endif
