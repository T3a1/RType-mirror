/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ShotStyles
*/

#ifndef SERVER_COMPONENTS_SHOTSTYLES_HPP
#define SERVER_COMPONENTS_SHOTSTYLES_HPP

#include <cstdint>

namespace server {

// Shot styles: how a player's weapon turns a shot (Weapon::fired) into
// projectiles. A player has exactly one of these components, applied by its
// own system (systems/ShotStyleSystems.hpp); power-ups give or upgrade them.
// See docs/WEAPONS.md to add a style.

/**
 * @struct SpreadShot
 * @brief Fires 2 * level - 1 shots at once, fanned out: a single shot at
 * level 1. Every ship starts with it.
 * @param level Its level, from 1 to WeaponStats::maxLevel.
 **/
struct SpreadShot {
    std::uint8_t level = 1;
};

/**
 * @enum PowerUpKind
 * @brief The shot style a power-up gives.
 * @param Spread A SpreadShot, or one more level of it.
 **/
enum class PowerUpKind : std::uint8_t {
    Spread,
};

} // namespace server

#endif // SERVER_COMPONENTS_SHOTSTYLES_HPP
