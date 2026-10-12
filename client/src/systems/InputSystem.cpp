/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** InputSystem
*/

#include "systems/InputSystem.hpp"
#include "common/components/InputState.hpp"
#include "engine/Entity.hpp"

#include <raylib.h>

#include <array>
#include <cstdint>

namespace client {

namespace {

struct KeyBinding {
    KeyboardKey key;
    std::uint8_t action;
};

constexpr std::array bindings{
    KeyBinding{.key = KEY_UP, .action = rtype::Action::Up},
    KeyBinding{.key = KEY_DOWN, .action = rtype::Action::Down},
    KeyBinding{.key = KEY_LEFT, .action = rtype::Action::Left},
    KeyBinding{.key = KEY_RIGHT, .action = rtype::Action::Right},
    KeyBinding{.key = KEY_SPACE, .action = rtype::Action::Fire},
};

} // namespace

void inputSystem(engine::Registry &registry)
{
    // IsKeyDown is true on every frame while the key is held: we want the
    // full state, not the press/release events.
    std::uint8_t actions = 0;
    for (const KeyBinding &binding : bindings) {
        if (IsKeyDown(binding.key)) {
            actions |= binding.action;
        }
    }

    // Only the local player has an InputState on the client.
    registry.each<rtype::InputState>(
        [actions](engine::Entity, rtype::InputState &input) {
            input.actions = actions;
        });
}

} // namespace client
