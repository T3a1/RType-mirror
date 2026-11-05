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

/** @brief The bits of Collider::layer and Collider::mask. */
namespace CollisionLayer {
/** @brief Touches nothing. */
inline constexpr std::uint32_t None = 0U;
/** @brief A player's ship. */
inline constexpr std::uint32_t Player = 1U << 0U;
/** @brief An enemy. */
inline constexpr std::uint32_t Enemy = 1U << 1U;
/** @brief A shot fired by a player. */
inline constexpr std::uint32_t PlayerProjectile = 1U << 2U;
/** @brief A shot fired by an enemy. */
inline constexpr std::uint32_t EnemyProjectile = 1U << 3U;
/** @brief A part of the level that blocks shots. */
inline constexpr std::uint32_t Obstacle = 1U << 4U;
/** @brief Something a player can collect. */
inline constexpr std::uint32_t Pickup = 1U << 5U;
} // namespace CollisionLayer

/**
 * @struct Collider
 * @brief An axis-aligned box centered on Transform::position.
 *
 * Two entities A and B collide if (A.layer & B.mask) != 0, e.g. a player
 * projectile has layer PlayerProjectile and mask Enemy | Obstacle.
 * @param size The width and height of the box, in playfield units.
 * @param layer What the entity is, as CollisionLayer bits.
 * @param mask What the entity touches, as CollisionLayer bits.
 **/
struct Collider {
    engine::Vec2f size;
    std::uint32_t layer = CollisionLayer::None;
    std::uint32_t mask = CollisionLayer::None;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_COLLIDER_HPP
