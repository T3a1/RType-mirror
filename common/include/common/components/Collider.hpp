/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Collider
*/

#ifndef COMMON_COMPONENTS_COLLIDER_HPP
#define COMMON_COMPONENTS_COLLIDER_HPP

#include "engine/Vec2.hpp"

#include <cstdint>

namespace rtype {

namespace CollisionLayer {
inline constexpr std::uint32_t None = 0U;
inline constexpr std::uint32_t Player = 1U << 0U;
inline constexpr std::uint32_t Enemy = 1U << 1U;
inline constexpr std::uint32_t PlayerProjectile = 1U << 2U;
inline constexpr std::uint32_t EnemyProjectile = 1U << 3U;
inline constexpr std::uint32_t Obstacle = 1U << 4U;
} // namespace CollisionLayer

// Axis-aligned box centered on Transform::position, in playfield units.
// Two entities A and B collide if (A.layer & B.mask) != 0, e.g. a player
// projectile has layer PlayerProjectile and mask Enemy | Obstacle.
struct Collider {
    engine::Vec2f size;
    std::uint32_t layer = CollisionLayer::None;
    std::uint32_t mask = CollisionLayer::None;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_COLLIDER_HPP
