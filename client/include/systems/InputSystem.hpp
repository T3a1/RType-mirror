/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** InputSystem
*/

#ifndef CLIENT_SYSTEMS_INPUTSYSTEM_HPP
#define CLIENT_SYSTEMS_INPUTSYSTEM_HPP

#include "Controls.hpp"
#include "engine/Clock.hpp"
#include "graphics/IGraphics.hpp"

#include <cstdint>

namespace client {

/**
 * @struct AutoFire
 * @brief The auto-fire switch: while it is on, the client holds the Fire
 * action and the server fires at its own rate. It is the only way to fire
 * normal shots: the fire key only charges.
 * @param enabled Whether auto-fire is on.
 * @param keyHeld Whether an AutoFire key was held last frame, so that
 * holding it switches only once.
 **/
struct AutoFire {
    bool enabled = true;
    bool keyHeld = false;
};

/**
 * @brief Tells whether one of the keys of a control is held, on the keyboard
 * or on the gamepad.
 * @param graphics The backend reading the keyboard.
 * @param controls The keys of each control.
 * @param control The control.
 * @return `true` if one of its keys is held.
 **/
[[nodiscard]] bool isHeld(const gfx::IGraphics &graphics,
                          const PlayerControls &controls, Control control);

/**
 * @brief Reads the keyboard through the player's controls.
 * @param graphics The graphics backend, for the keyboard.
 * @param controls The keys bound to each control.
 * @param autoFire The auto-fire switch, flipped when an AutoFire key is
 * pressed.
 * @return The actions held this frame, as Action bits: the fire key gives
 * Charge, and Fire is held while auto-fire is on.
 **/
[[nodiscard]] std::uint8_t inputSystem(const gfx::IGraphics &graphics,
                                       const PlayerControls &controls,
                                       AutoFire &autoFire);

} // namespace client

#endif // CLIENT_SYSTEMS_INPUTSYSTEM_HPP
