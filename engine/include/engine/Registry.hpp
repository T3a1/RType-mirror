/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Registry
*/

#ifndef ENGINE_REGISTRY_HPP
#define ENGINE_REGISTRY_HPP

#include "engine/Entity.hpp"
#include "engine/SparseSet.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace engine {

/**
 * @class Registry
 * @brief Owns the entities and one storage per component type, and answers
 * queries such as "every entity with a Transform and a Velocity".
 *
 * Not thread-safe: each game uses its registry on one thread.
 **/
class Registry {
  public:
    /**
     * @brief Adds a component to an entity, replacing the one it may already
     *have.
     * @tparam C The component type.
     * @param entity The entity.
     * @param args What to initialize the component with.
     * @return The component in its storage. Adding or removing a C invalidates
     *it.
     **/
    template <typename C, typename... Args>
    C &emplace(Entity entity, Args &&...args)
    {
        return storage<C>().emplace(entity, std::forward<Args>(args)...);
    }

    /**
     * @brief Returns a component the entity must have; checked with an assert
     *only.
     * @tparam C The component type.
     * @param entity The entity.
     * @return The component.
     **/
    template <typename C> C &get(Entity entity)
    {
        return storage<C>().get(entity);
    }

    /**
     * @brief Returns a component the entity may not have.
     * @tparam C The component type.
     * @param entity The entity.
     * @return The component, nullptr if the entity does not have it.
     **/
    template <typename C> C *tryGet(Entity entity)
    {
        return storage<C>().tryGet(entity);
    }

    /**
     * @brief Tells whether an entity has a component.
     * @tparam C The component type.
     * @param entity The entity.
     * @return `true` if it has it.
     **/
    template <typename C> [[nodiscard]] bool has(Entity entity)
    {
        return storage<C>().contains(entity);
    }

    /**
     * @brief Removes a component from an entity, if it has it.
     * @tparam C The component type.
     * @param entity The entity.
     **/
    template <typename C> void remove(Entity entity)
    {
        storage<C>().remove(entity);
    }

    /**
     * @brief Calls a function for every entity that has all the given
     *components.
     *
     * Walks the storage of First and checks the others, so First should be the
     * rarest. Do not add or remove First during the walk: request it for later.
     * @tparam First The first component type.
     * @tparam Others The other component types.
     * @param func Called with the entity, then a reference to each component.
     **/
    template <typename First, typename... Others, typename Func>
    void each(Func func)
    {
        SparseSet<First> &first = storage<First>();
        for (std::size_t i = 0; i < first.size(); ++i) {
            const Entity entity = first.entityAt(i);
            if ((storage<Others>().contains(entity) && ...)) {
                func(entity, first.dataAt(i), storage<Others>().get(entity)...);
            }
        }
    }

    /**
     * @brief Creates an entity, reusing a free index with a new generation.
     * @return The new entity, with no component.
     **/
    [[nodiscard]] Entity create();

    /**
     * @brief Marks an entity for destruction at the next flush(), so that
     *systems can destroy entities during a walk. Does nothing if it is not
     *alive.
     * @param entity The entity.
     **/
    void destroy(Entity entity);

    /**
     * @brief Destroys the marked entities: removes their components and frees
     *their index. The scheduler calls it after the Cleanup stage.
     **/
    void flush();
    /**
     * @brief Tells whether a handle still refers to a living entity.
     * @param entity The entity.
     * @return `true` if it exists, including when marked for destruction but
     *not flushed yet; `false` for a destroyed entity, even if its index was
     *reused.
     **/
    [[nodiscard]] bool alive(Entity entity) const;

  private:
    template <typename C> SparseSet<C> &storage()
    {
        std::unique_ptr<ISparseSet> &slot =
            m_storages[std::type_index(typeid(C))];
        if (!slot) {
            slot = std::make_unique<SparseSet<C>>();
        }
        return static_cast<SparseSet<C> &>(*slot);
    }

    std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>> m_storages;
    std::vector<std::uint32_t> m_generations;
    std::vector<std::uint32_t> m_freeIndices;
    std::vector<Entity> m_toDestroy;
};

} // namespace engine

#endif // ENGINE_REGISTRY_HPP
