/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** InputSystem
*/

#include "systems/InputSystem.hpp"
#include "common/components/InputState.hpp"

#include <array>
#include <cstdint>

namespace client {

namespace {

struct KeyBinding {
    gfx::Key key;
    std::uint8_t action;
};

constexpr std::array bindings{
    KeyBinding{.key = gfx::Key::Up, .action = rtype::Action::Up},
    KeyBinding{.key = gfx::Key::Down, .action = rtype::Action::Down},
    KeyBinding{.key = gfx::Key::Left, .action = rtype::Action::Left},
    KeyBinding{.key = gfx::Key::Right, .action = rtype::Action::Right},
    KeyBinding{.key = gfx::Key::Space, .action = rtype::Action::Fire},
};

} // namespace

std::uint8_t inputSystem(const gfx::IGraphics &graphics)
{
    std::uint8_t actions = 0;
    for (const KeyBinding &binding : bindings) {
        if (graphics.isKeyDown(binding.key)) {
            actions |= binding.action;
        }
    }
    return actions;
}

} // namespace client
