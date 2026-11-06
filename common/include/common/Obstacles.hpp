/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Obstacles
*/

#ifndef COMMON_OBSTACLES_HPP
#define COMMON_OBSTACLES_HPP

#include "engine/Vec2.hpp"

namespace rtype {

/**
 * @brief The size of one wall block or destroyable tile, in playfield units.
 * Obstacles are laid out on a grid of this size.
 */
inline constexpr engine::Vec2f TileSize{.x = 64.0F, .y = 64.0F};

/**
 * @brief The full health of a destroyable tile: three normal shots break it.
 */
inline constexpr int TileMaxHealth = 150;

/**
 * @brief How fast the obstacles scroll to the left, in playfield units per
 * second.
 */
inline constexpr float ObstacleSpeed = 120.0F;

} // namespace rtype

#endif // COMMON_OBSTACLES_HPP
