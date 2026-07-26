#pragma once

#if KRIT_ENABLE_ECS

#include "krit/utils/FindFirst.h"

namespace krit {

struct World;

template <typename WorldType = World> struct System {
    virtual void update(WorldType *world);
};

}

#endif
