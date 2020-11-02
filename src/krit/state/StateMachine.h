#ifndef KRIT_STATE_STATEMACHINE
#define KRIT_STATE_STATEMACHINE

#include <memory>
#include <vector>

struct StateMachineState {
    virtual void onEnter(int state) {}
    virtual void onExit(int state) {}
};

struct StateMachine {
    int state = -1;
    std::vector<std::unique_ptr<StateMachineState>> states;

    StateMachine() {}

    void setState(int index) {
        if (state != index) {
            if (state != -1) {
                states[state]->onExit(state);
            }
            state = index;
            states[state]->onEnter(state);
        }
    }

    void addState(int index, StateMachineState *state) {
        addState(index, std::move(std::unique_ptr<StateMachineState>(state)));
    }
    void addState(int index, std::unique_ptr<StateMachineState> state) {
        if (states.size() < static_cast<size_t>(index + 1)) {
            states.resize(index + 1);
        }
        states[index] = std::move(state);
    }
};

#endif
