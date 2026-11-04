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
 * @brief The auto-fire switch: while it is on, the ship fires without a
 * key held.
 * @param enabled Whether auto-fire is on.
 * @param keyHeld Whether an AutoFire key was held last frame, so that
 * holding it switches only once.
 * @param elapsed The time since auto-fire was turned on, in seconds: it
 * presses and releases Fire in turn, since the server fires on each press.
 **/
struct AutoFire {
    bool enabled = false;
    bool keyHeld = false;
    float elapsed = 0.0F;
};

/** @brief How often auto-fire presses Fire, in seconds: the default time
 * between two shots. */
inline constexpr float AutoFirePeriod = 0.25F;

/**
 * @brief Reads the keyboard through the player's controls.
 * @param graphics The graphics backend, for the keyboard.
 * @param controls The keys bound to each control.
 * @param autoFire The auto-fire switch, flipped when an AutoFire key is
 * pressed.
 * @param dt The time elapsed since the last frame.
 * @return The actions held this frame, as Action bits. While auto-fire is
 * on, Fire is pressed during the first half of every AutoFirePeriod and
 * released during the second half.
 **/
[[nodiscard]] std::uint8_t inputSystem(const gfx::IGraphics &graphics,
                                       const Controls &controls,
                                       AutoFire &autoFire, engine::Duration dt);

} // namespace client

#endif // CLIENT_SYSTEMS_INPUTSYSTEM_HPP
