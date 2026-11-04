/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** InputSystem
*/

#ifndef CLIENT_SYSTEMS_INPUTSYSTEM_HPP
#define CLIENT_SYSTEMS_INPUTSYSTEM_HPP

#include "Controls.hpp"
#include "graphics/IGraphics.hpp"

#include <cstdint>

namespace client {

/**
 * @struct AutoFire
 * @brief The auto-fire switch: while it is on, the ship fires without a
 * key held.
 * @param enabled Whether auto-fire is on.
 * @param keyHeld Whether an AutoFire key was held last frame, so that
 * holding it switches only once.
 **/
struct AutoFire {
    bool enabled = false;
    bool keyHeld = false;
};

/**
 * @brief Reads the keyboard through the player's controls.
 * @param graphics The graphics backend, for the keyboard.
 * @param controls The keys bound to each control.
 * @param autoFire The auto-fire switch, flipped when an AutoFire key is
 * pressed.
 * @return The actions held this frame, as Action bits (Fire is held while
 * auto-fire is on).
 **/
[[nodiscard]] std::uint8_t inputSystem(const gfx::IGraphics &graphics,
                                       const Controls &controls,
                                       AutoFire &autoFire);

} // namespace client

#endif // CLIENT_SYSTEMS_INPUTSYSTEM_HPP
