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
 * @struct Weapon
 * @brief The entity can shoot projectiles.
 * @param fireInterval The time between two shots.
 * @param cooldown The time left before the next shot.
 * @param triggered Whether the weapon is currently shooting or not.
 * @param charging Whether a player holds the Charge action.
 * @param charge How long the Charge action has been held, for the
 * charged shot of a player.
 * @param fired Whether a player's weapon fires this tick, set by the
 * shoot system; the system of the player's shot style turns it into
 * projectiles.
 **/
struct Weapon {
    engine::Duration fireInterval{0.25F};
    engine::Duration cooldown{0.0F};
    bool triggered = false;
    bool charging = false;
    engine::Duration charge{0.0F};
    bool fired = false;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_WEAPON_HPP
