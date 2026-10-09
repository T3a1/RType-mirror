/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Registry
*/
#include "engine/Registry.hpp"

#include <cstdint>

namespace engine {

Entity Registry::create()
{
    std::uint32_t id = 0;
    if (!m_freeIndices.empty()) {
        id = m_freeIndices.back();
        m_freeIndices.pop_back();
    } else {
        id = static_cast<std::uint32_t>(m_generations.size());
        m_generations.push_back(0);
    }
    return Entity{.id = id, .generation = m_generations[id]};
}

void Registry::destroy(Entity entity)
{
    if (alive(entity)) {
        m_toDestroy.push_back(entity);
    }
}

void Registry::flush()
{
    for (const Entity entity : m_toDestroy) {
        if (!alive(entity)) {
            continue;
        }
        for (auto &[type, set] : m_storages) {
            set->remove(entity);
        }
        ++m_generations[entity.id];
        m_freeIndices.push_back(entity.id);
    }
    m_toDestroy.clear();
}

bool Registry::alive(Entity entity) const
{
    if (entity.id >= m_generations.size()) {
        return false;
    }
    return entity.generation == m_generations[entity.id];
}

} // namespace engine
