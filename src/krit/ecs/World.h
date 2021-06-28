#ifndef KRIT_ECS_WORLD
#define KRIT_ECS_WORLD

#include "krit/ecs/Entity.h"
#include "krit/ecs/Utils.h"
#include <queue>
#include <unordered_map>

namespace krit {

template <typename... Components> struct World {
    unsigned int count;

    /**
     * Returns a map of all entities that have a specific component.
     */
    template <typename ComponentType>
    std::unordered_map<EntityId, ComponentType> &getAll() {
        return std::get<
            find_first<std::tuple<Components...>, ComponentType>::value>(
            this->components);
    }

    /**
     * Add an EntityTemplate to the world. Creates a new entity and adds all
     * components from the template.
     */
    template <typename... TemplateComponents>
    void add(EntityTemplate<TemplateComponents...> &e) {
        EntityId id = this->newEntity();
        this->_addAll<EntityTemplate<TemplateComponents...>,
                      TemplateComponents...>(e, id, TemplateComponents()...);
        allEntities[id] = true;
    }

    /**
     * Add a single component attached to an existing entity.
     */
    template <typename ComponentType, class... Args>
    ComponentType &add(const Entity &e, Args &&... args) {
        return this->add<ComponentType, Args...>(args...);
    }
    template <typename ComponentType, class... Args>
    ComponentType &add(EntityId e, Args &&... args) {
        auto &map = std::get<
            find_first<std::tuple<Components...>, ComponentType>::value>(
            this->components);
        map.emplace(std::piecewise_construct, std::forward_as_tuple(e),
                    std::forward_as_tuple(args...));
        return map[e];
    }

    /**
     * Returns `true` if the entity has the specified component attached.
     */
    template <typename ComponentType> bool has(const Entity &e) {
        return this->has<ComponentType>(e.id);
    }
    template <typename ComponentType> bool has(EntityId e) {
        auto &map = std::get<
            find_first<std::tuple<Components...>, ComponentType>::value>(
            this->components);
        auto it = map.find(e);
        return it != map.end();
    }

    /**
     * Returns a reference to the entity's specific component.
     */
    template <typename ComponentType> ComponentType &get(const Entity &e) {
        return this->get<ComponentType>(e.id);
    }
    template <typename ComponentType> ComponentType &get(EntityId e) {
        auto &map = std::get<
            find_first<std::tuple<Components...>, ComponentType>::value>(
            this->components);
        return map[e];
    }

    /**
     * Returns a pointer to the component, which can be null if it doesn't
     * exist.
     */
    template <typename ComponentType> ComponentType *maybeGet(const Entity &e) {
        return this->maybeGet<ComponentType>(e.id);
    }
    template <typename ComponentType> ComponentType *maybeGet(EntityId e) {
        auto &map = std::get<
            find_first<std::tuple<Components...>, ComponentType>::value>(
            this->components);
        auto it = map.find(e);
        return it == map.end() ? nullptr : &it->second;
    }

    /**
     * Remove a specific component from an entity.
     */
    template <typename ComponentType> void remove(const Entity &e) {
        this->remove<ComponentType>(e.id);
    }
    template <typename ComponentType> void remove(EntityId e) {
        auto &map = std::get<
            find_first<std::tuple<Components...>, ComponentType>::value>(
            this->components);
        map.erase(e);
    }

    /**
     * Removes all components attached to this entity.
     */
    void removeAll(const Entity &e) { this->removeAll(e.id); }
    void removeAll(EntityId e) {
        --count;
        this->_removeAll<Components...>(e, Components()...);
        this->recycleList.push_back(e);
        allEntities.erase(e);
    }

    EntityId newEntity() {
        ++count;
        EntityId e;
        if (this->freeList.empty()) {
            e = this->next++;
        } else {
            e = this->freeList.back();
            this->freeList.pop_back();
        }
        return e;
    }

    bool exists(EntityId e) { return allEntities.find(e) != allEntities.end(); }

    void clear() {
        this->_clearAll<Components...>(Components()...);
        this->next = Entity::NO_ENTITY + 1;
        freeList.clear();
        recycleList.clear();
        allEntities.clear();
    }

    void recycleEntities() {
        std::move(this->recycleList.begin(), this->recycleList.end(),
                  std::back_inserter(this->freeList));
        this->recycleList.clear();
    }

private:
    std::unordered_map<EntityId, bool> allEntities;
    std::tuple<std::unordered_map<EntityId, Components>...> components;
    EntityId next = Entity::NO_ENTITY + 1;
    std::vector<EntityId> freeList, recycleList;

    template <typename T, typename Head, typename... Tail>
    void _addAll(T &e, EntityId id, Head head, Tail... tail) {
        this->_addAll<T, Head>(e, id, head);
        this->_addAll<T, Tail...>(e, id, tail...);
    }

    template <typename T, typename Head> void _addAll(T &e, EntityId id, Head) {
        auto &map =
            std::get<find_first<std::tuple<Components...>, Head>::value>(
                this->components);
        map.insert(std::make_pair(id, e.template get<Head>()));
    }

    template <typename Head, typename... Tail>
    void _removeAll(EntityId e, Head head, Tail... tail) {
        this->_removeAll<Head>(e, head);
        this->_removeAll<Tail...>(e, tail...);
    }
    template <typename Head> void _removeAll(EntityId e, Head) {
        this->remove<Head>(e);
    }

    template <typename Head, typename... Tail>
    void _clearAll(Head head, Tail... tail) {
        this->_clearAll<Head>(head);
        this->_clearAll<Tail...>(tail...);
    }
    template <typename Head> void _clearAll(Head) {
        auto &map =
            std::get<find_first<std::tuple<Components...>, Head>::value>(
                this->components);
        map.clear();
    }
};

}

#endif
