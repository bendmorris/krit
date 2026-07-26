#pragma once

#if KRIT_ENABLE_ECS

#include "krit/utils/FindFirst.h"
#include <tuple>

namespace krit {

typedef uint32_t EntityId;

struct Entity {
    static const int NO_ENTITY = 0;
    EntityId id;

    Entity(EntityId id) : id(id) {}
};

template <typename... Components> struct EntityTemplate {
    std::tuple<Components...> components;

    template <typename ComponentType> ComponentType &get() {
        return std::get<
            find_first<std::tuple<Components...>, ComponentType>::value>(
            this->components);
    }
};

}

#endif
