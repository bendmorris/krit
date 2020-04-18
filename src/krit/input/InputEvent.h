#ifndef KRIT_INPUT_LEVEL
#define KRIT_INPUT_LEVEL

#include "krit/Math.h"
#include <string>

using namespace std;
using namespace krit;

namespace krit {

typedef int InputType;
typedef int Action;

union LevelData {
    bool active;
    int level;
    Point position;

    LevelData() : active(false) {}
    LevelData(bool active) : active(active) {}
    LevelData(int level) : level(level) {}
    LevelData(Point &position) : position(position) {}
};

enum InputEventType {
    InputStart,
    InputActive,
    InputFinish,
};

struct InputEvent {
    InputEventType eventType;
    Action action;
    LevelData level;

    InputEvent(InputEventType eventType, Action action, LevelData level)
        : eventType(eventType), action(action), level(level) {}
};

}

#endif
