#ifndef KRIT_ECS_ENTITY
#define KRIT_ECS_ENTITY

#include "krit/ecs/Utils.h"
#include <tuple>

namespace krit {

typedef uint16_t EntityId;

struct Entity {
    static const int NO_ENTITY = 0;
    EntityId id;

    Entity(EntityId id): id(id) {}
};

template <typename... Components> struct EntityTemplate {
    std::tuple<Components...> components;

    template <typename ComponentType> ComponentType &get() {
        return std::get<find_first<std::tuple<Components...>, ComponentType>::value>(this->components);
    }
};

}

#endif
