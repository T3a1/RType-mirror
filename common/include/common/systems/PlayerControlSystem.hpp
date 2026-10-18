/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PlayerControlSystem
*/

#ifndef COMMON_SYSTEMS_PLAYERCONTROLSYSTEM_HPP
#define COMMON_SYSTEMS_PLAYERCONTROLSYSTEM_HPP

#include "common/components/InputState.hpp"
#include "common/components/Weapon.hpp"
#include "engine/Registry.hpp"

namespace rtype {

// Speed of a player ship, in playfield units per second.
inline constexpr float playerSpeed = 300.0F;

// Turns the movement bits of each InputState into the entity's Velocity.
void playerControlSystem(engine::Registry &registry);
void playerFireSystem(engine::Registry &registry);

} // namespace rtype

#endif // COMMON_SYSTEMS_PLAYERCONTROLSYSTEM_HPP
