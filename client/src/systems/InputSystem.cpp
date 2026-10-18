/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** InputSystem
*/

#include "systems/InputSystem.hpp"
#include "common/components/InputState.hpp"

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

std::uint8_t inputSystem()
{
    std::uint8_t actions = 0;
    for (const KeyBinding &binding : bindings) {
        if (IsKeyDown(binding.key)) {
            actions |= binding.action;
        }
    }
    return actions;
}

} // namespace client
