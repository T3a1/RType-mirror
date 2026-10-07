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

    // using Entity = std::uint32_t;
    // const Entity MAX_ENTITIES = 5000;

    struct Entity {
        std::uint32_t id;
        std::uint32_t generation;

        friend constexpr bool operator==(Entity, Entity) = default;
    };

} // namespace engine

#endif // ENGINE_ENTITY_HPP
