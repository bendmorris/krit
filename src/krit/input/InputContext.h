#ifndef KRIT_INPUT_INPUT
#define KRIT_INPUT_INPUT

#include "krit/utils/Signal.h"
#include "krit/input/ControlBindings.h"
#include "krit/input/InputEvent.h"
#include "krit/input/Key.h"
#include "krit/input/Mouse.h"
#include "krit/UpdateContext.h"
#include <string>
#include <unordered_map>
#include <utility>

using namespace std;
using namespace krit;

namespace krit {

struct ActionContext {
    const Action action = 0;
    LevelData level;

    ActionContext(Action action, LevelData level): action(action), level(level) {}
};

typedef Signal2<ActionContext*, UpdateContext*> InputSignal;

struct InputCallbacks {
    InputSignal onStart;
    InputSignal onActive;
    InputSignal onFinish;
    double delay = 0;
    double timer = 0;

    InputCallbacks(InputSignal onStart, InputSignal onActive, InputSignal onFinish, double delay = 0)
        : onStart(onStart), onActive(onActive), onFinish(onFinish), delay(delay) {}
};

struct InputContext {
    bool enabled = true;
    UpdateSignal onUpdate = nullptr;

    unordered_map<Action, InputCallbacks> actionMappings;
    unordered_map<InputType, bool> actionStates;
    unordered_map<InputType, LevelData> active;

    ControlBindings *bindings;

    InputContext(ControlBindings *bindings): bindings(bindings) {}

    void update(UpdateContext &ctx);

    void bind(Action action, InputSignal onStart = nullptr, InputSignal onActive = nullptr, InputSignal onFinish = nullptr, double delay = 0) {
        this->actionMappings.insert(make_pair(action, InputCallbacks(onStart, onActive, onFinish, delay)));
    }

    void unbind(Action action) {
        this->actionMappings.erase(action);
    }

    void clear() {
        this->actionMappings.clear();
    }

    bool isActive(Action action) {
        return this->active.find(action) != this->active.end();
    }

    void activate(Action action, LevelData &l) {
        this->active.insert(make_pair(action, l));
    }
};

}
#endif
