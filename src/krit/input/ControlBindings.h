#ifndef KRIT_INPUT_CONTROLBINDING
#define KRIT_INPUT_CONTROLBINDING

#include "krit/input/InputEvent.h"
#include "krit/input/Key.h"
#include "krit/input/Mouse.h"
#include "krit/UpdateContext.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;
using namespace krit;

namespace krit {

class ControlBindings {
    public:
        KeyManager key;
        MouseManager mouse;

        ControlBindings(): key(&this->events), mouse(&this->events) {}

        void reset() {
            this->mouse.reset();
        }

        void update(UpdateContext &ctx) {
            this->key.update();
            this->mouse.update();
        }

        void clear() {
            this->events.clear();
        }

        void bindKey(Key key, Action action) {
            this->key.define(key, action);
        }

        void unbindKey(Key key) {
            this->key.undefine(key);
        }

        void bindMouse(MouseButton btn, Action action) {
            this->mouse.define(btn, action);
        }

        void unbindMouse(MouseButton btn) {
            this->mouse.undefine(btn);
        }

        vector<InputEvent> &getEvents() { return this->events; }

        friend class MouseManager;
        friend class KeyManager;

    private:
        vector<InputEvent> events;
};

}
#endif
