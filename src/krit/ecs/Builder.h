#ifndef KRIT_ECS_BUILDER
#define KRIT_ECS_BUILDER

#include "krit/ecs/Entity.h"
#include "krit/ecs/World.h"

namespace krit {

template <typename... WorldComponents> struct Builder;

template <typename... WorldComponents, typename... Components>
struct Builder<std::tuple<WorldComponents...>, Components...>
    : struct EntityTemplate<WorldComponents...> {
    template <typename Component, class... Args>
    &Builder<std::tuple<WorldComponents...>, Components..., Component>
    add(Args &&... args) {
        this->get<Component>() = Component(args...);
        return *this;
    }

    EntityTemplate<Components...> build() {
        EntityTemplate<Components...> t;
        this->_buildTemplate<std::tuple<Components...>, Components...>(t);
        return t;
    }

private:
    template <typename AllComponents..., typename Head, typename Tail...>
    void _buildTemplate<std::tuple<AllComponents...>, Head, Tail...>(
        EntityTemplate<AllComponents...> &t) {
        this->_buildTemplate<std::tuple<AllComponents...>, Head>(t);
        this->_buildTemplate<std::tuple<AllComponents...>, Tail...>(t);
    }

    template <typename AllComponents..., typename Head>
    void _buildTemplate<std::tuple<AllComponents...>, Head>(
        EntityTemplate<AllComponents...> &t) {
        t.get<Head>() = this->get<Head>();
    }
};

}

#endif
