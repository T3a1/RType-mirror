/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** InputSystem
*/

#include "systems/InputSystem.hpp"
#include "common/components/InputState.hpp"

#include <algorithm>
#include <array>
#include <cstdint>

namespace client {

namespace {

/**
 * @struct ActionBinding
 * @brief The action a control gives to the server.
 * @param control The control.
 * @param action Its Action bit.
 **/
struct ActionBinding {
    Control control;
    std::uint8_t action;
};

/** @brief The controls that are sent to the server. */
constexpr std::array ActionBindings{
    ActionBinding{.control = Control::Up, .action = rtype::Action::Up},
    ActionBinding{.control = Control::Down, .action = rtype::Action::Down},
    ActionBinding{.control = Control::Left, .action = rtype::Action::Left},
    ActionBinding{.control = Control::Right, .action = rtype::Action::Right},
    ActionBinding{.control = Control::Fire, .action = rtype::Action::Charge},
};

} // namespace

bool isHeld(const gfx::IGraphics &graphics, const PlayerControls &controls,
            Control control)
{
    const auto down = [&graphics](gfx::Key key) {
        return graphics.isKeyDown(key);
    };
    const auto index = static_cast<std::size_t>(control);
    return std::ranges::any_of(controls.keyboard.at(index), down) ||
           std::ranges::any_of(controls.gamepad.at(index), down);
}

std::uint8_t inputSystem(const gfx::IGraphics &graphics,
                         const PlayerControls &controls, AutoFire &autoFire)
{
    const bool toggle = isHeld(graphics, controls, Control::AutoFire);
    if (toggle && !autoFire.keyHeld) {
        autoFire.enabled = !autoFire.enabled;
    }
    autoFire.keyHeld = toggle;

    std::uint8_t actions = 0;
    for (const ActionBinding &binding : ActionBindings) {
        if (isHeld(graphics, controls, binding.control)) {
            actions |= binding.action;
        }
    }
    if (autoFire.enabled) {
        actions |= rtype::Action::Fire;
    }
    return actions;
}

} // namespace client
