/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Weapon
*/

#ifndef COMMON_COMPONENTS_WEAPON_HPP
#define COMMON_COMPONENTS_WEAPON_HPP

#include "engine/Clock.hpp"

namespace rtype {

/**
 * @brief The entity can shoot projectiles.
 * @param fireInterval The time between two shots.
 * @param cooldown The time left before the next shot.
 * @param triggered Whether the weapon is currently shooting or not.
 **/
struct Weapon {
    engine::Duration fireInterval{0.25F};
    engine::Duration cooldown{0.0F};
    bool triggered = false;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_WEAPON_HPP
