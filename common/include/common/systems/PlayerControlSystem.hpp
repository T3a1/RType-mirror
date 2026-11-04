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
#include "engine/Vec2.hpp"

#include <cstdint>

namespace rtype {

// Speed of a player ship, in playfield units per second.
inline constexpr float playerSpeed = 500.0F;

/**
 * @brief Returns the velocity of a player ship holding these actions: the
 * Up, Down, Left and Right bits at playerSpeed, as fast diagonally.
 * @param actions The held actions, as Action bits.
 * @return The velocity, in playfield units per second.
 **/
engine::Vec2f shipVelocity(std::uint8_t actions);

// Turns the movement bits of each InputState into the entity's Velocity.
void playerControlSystem(engine::Registry &registry);
void playerFireSystem(engine::Registry &registry);

} // namespace rtype

#endif // COMMON_SYSTEMS_PLAYERCONTROLSYSTEM_HPP
