/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PlayerControlSystem
*/

#include "common/systems/PlayerControlSystem.hpp"
#include "common/components/InputState.hpp"
#include "common/components/Velocity.hpp"
#include "engine/Entity.hpp"

#include <cstdint>
#include <numbers>

namespace rtype {

namespace {

// -1, 0 or 1 along one axis. Both keys held cancel each other out.
float axis(std::uint8_t actions, std::uint8_t negative, std::uint8_t positive)
{
    float value = 0.0F;
    if ((actions & negative) != 0) {
        value -= 1.0F;
    }
    if ((actions & positive) != 0) {
        value += 1.0F;
    }
    return value;
}

} // namespace

void playerControlSystem(engine::Registry &registry)
{
    registry.each<InputState, Velocity>(
        [](engine::Entity, const InputState &input, Velocity &velocity) {
            engine::Vec2f direction{
                .x = axis(input.actions, Action::Left, Action::Right),
                .y = axis(input.actions, Action::Up, Action::Down),
            };
            // A diagonal (1, 1) is sqrt(2) long: shrink it back to 1 so
            // the ship is not faster diagonally.
            if (direction.x != 0.0F && direction.y != 0.0F) {
                direction *= 1.0F / std::numbers::sqrt2_v<float>;
            }
            velocity.value = direction * playerSpeed;
        });
}

/**
 * Arms the weapon of each player holding the Fire action.
 **/
void playerFireSystem(engine::Registry &registry)
{
    registry.each<Weapon, rtype::InputState>(
        [](engine::Entity, Weapon &weapon, const rtype::InputState &input) {
            weapon.triggered = (input.actions & rtype::Action::Fire) != 0;
        });
}

} // namespace rtype
