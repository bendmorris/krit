#ifndef KRIT_ECS_SYSTEM
#define KRIT_ECS_SYSTEM

#include "krit/ecs/Utils.h"

namespace krit {

struct World;

template <typename WorldType = World> struct System {
    virtual void update(WorldType *world);
};

}

#endif
