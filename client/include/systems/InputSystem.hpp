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
    bool enabled = true;
    bool keyHeld = false;
    float elapsed = 0.0F;
};

/** @brief How often auto-fire presses Fire, in seconds: a little more than
 * the default time between two shots, so that the server never ignores a
 * press because the last shot is still cooling down. */
inline constexpr float AutoFirePeriod = 0.2F;

/**
 * @brief Tells whether one of the keys of a control is held.
 * @param graphics The backend reading the keyboard.
 * @param controls The keys of each control.
 * @param control The control.
 * @return `true` if one of its keys is held.
 **/
[[nodiscard]] bool isHeld(const gfx::IGraphics &graphics,
                          const Controls &controls, Control control);

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
