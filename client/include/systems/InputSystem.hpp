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
 * @return The actions held this frame, as Action bits: the fire key gives
 * Charge, and Fire is always held, since auto-fire is always on: the server
 * fires the normal shots at its own rate.
 **/
[[nodiscard]] std::uint8_t inputSystem(const gfx::IGraphics &graphics,
                                       const PlayerControls &controls);

} // namespace client

#endif // CLIENT_SYSTEMS_INPUTSYSTEM_HPP
