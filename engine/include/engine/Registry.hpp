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

class Registry {
  public:
    template <typename C, typename... Args>
    C &emplace(Entity entity, Args &&...args)
    {
        return storage<C>().emplace(entity, std::forward<Args>(args)...);
    }

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

    [[nodiscard]] Entity create();
    void destroy(Entity entity);
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
};

} // namespace engine

#endif // ENGINE_REGISTRY_HPP
