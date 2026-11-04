/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Weapon
*/

#ifndef COMMON_COMPONENTS_WEAPON_HPP
#define COMMON_COMPONENTS_WEAPON_HPP

#include "engine/Clock.hpp"

#include <cstdint>

namespace rtype {

/**
 * @brief The entity can shoot projectiles.
 * @param fireInterval The time between two shots.
 * @param cooldown The time left before the next shot.
 * @param triggered Whether the weapon is currently shooting or not.
 * @param level The upgrade level of a player's weapon, from 1: more shots
 * at once at each level. Raised by power-ups.
 * @param charge How long the fire key has been held, for the charged
 * shot of a player.
 **/
struct Weapon {
    engine::Duration fireInterval{0.25F};
    engine::Duration cooldown{0.0F};
    bool triggered = false;
    std::uint8_t level = 1;
    engine::Duration charge{0.0F};
};

} // namespace rtype

#endif // COMMON_COMPONENTS_WEAPON_HPP
