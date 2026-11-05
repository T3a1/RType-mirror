/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Entity
*/

#ifndef ENGINE_ENTITY_HPP
#define ENGINE_ENTITY_HPP
#include <cstdint>

namespace engine {

/**
 * @struct Entity
 * @brief A handle to an entity: an identifier with no data or behavior.
 * @param id The index of the entity in the storages. Indexes are reused.
 * @param generation Tells apart the entities that used this index, so that a
 * handle to a destroyed entity can be detected.
 **/
struct Entity {
    std::uint32_t id;
    std::uint32_t generation;

    friend constexpr bool operator==(Entity, Entity) = default;
};

} // namespace engine

#endif // ENGINE_ENTITY_HPP
