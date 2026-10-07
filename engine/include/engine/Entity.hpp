/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Entity
*/

#ifndef ENGINE_ENTITY_HPP
    #define ENGINE_ENTITY_HPP
    #include <cstdint>
    #include <functional>

namespace engine {

    // using Entity = std::uint32_t;
    // const Entity MAX_ENTITIES = 5000;

    struct Entity {
        std::uint32_t index;
        std::uint32_t generation;

        friend constexpr bool operator==(Entity, Entity) = default;
    };

} // namespace engine

template <> struct std::hash<engine::Entity> {
    std::size_t operator()(engine::Entity e) const noexcept
    {
        return (static_cast<std::uint64_t>(e.generation) << 32) | e.index;
    }
};

#endif // ENGINE_ENTITY_HPP
