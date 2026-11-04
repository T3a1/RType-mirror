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
#include <cmath>
#include <cstdint>

namespace client {

namespace {

struct ActionBinding {
    Control control;
    std::uint8_t action;
};

constexpr std::array ActionBindings{
    ActionBinding{.control = Control::Up, .action = rtype::Action::Up},
    ActionBinding{.control = Control::Down, .action = rtype::Action::Down},
    ActionBinding{.control = Control::Left, .action = rtype::Action::Left},
    ActionBinding{.control = Control::Right, .action = rtype::Action::Right},
    ActionBinding{.control = Control::Fire, .action = rtype::Action::Fire},
};

/** @brief Whether one of the keys of a control is held. */
bool isHeld(const gfx::IGraphics &graphics, const Controls &controls,
            Control control)
{
    return std::ranges::any_of(
        controls.at(static_cast<std::size_t>(control)),
        [&graphics](gfx::Key key) { return graphics.isKeyDown(key); });
}

} // namespace

std::uint8_t inputSystem(const gfx::IGraphics &graphics,
                         const Controls &controls, AutoFire &autoFire,
                         engine::Duration dt)
{
    const bool toggle = isHeld(graphics, controls, Control::AutoFire);
    if (toggle && !autoFire.keyHeld) {
        autoFire.enabled = !autoFire.enabled;
        autoFire.elapsed = 0.0F;
    }
    autoFire.keyHeld = toggle;

    std::uint8_t actions = 0;
    for (const ActionBinding &binding : ActionBindings) {
        if (isHeld(graphics, controls, binding.control)) {
            actions |= binding.action;
        }
    }
    if (autoFire.enabled) {
        // Press, release, press...: the server fires once per press.
        autoFire.elapsed =
            std::fmod(autoFire.elapsed + dt.count(), AutoFirePeriod);
        if (autoFire.elapsed < AutoFirePeriod / 2.0F) {
            actions |= rtype::Action::Fire;
        }
    }
    return actions;
}

} // namespace client
