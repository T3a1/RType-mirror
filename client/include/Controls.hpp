/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Controls
*/

#ifndef CLIENT_CONTROLS_HPP
#define CLIENT_CONTROLS_HPP

#include "graphics/Types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace client {

/**
 * @enum Control
 * @brief What the player can do with the keyboard.
 * @param Up Moves the ship up.
 * @param Down Moves the ship down.
 * @param Left Moves the ship to the left.
 * @param Right Moves the ship to the right.
 * @param Fire Fires, or charges a shot while held.
 * @param AutoFire Turns auto-fire on or off: the ship fires without a key held.
 * @param Count The number of controls, not a control.
 **/
enum class Control : std::uint8_t {
    Up,
    Down,
    Left,
    Right,
    Fire,
    AutoFire,
    Count,
};

/** @brief The number of controls. */
inline constexpr std::size_t ControlCount =
    static_cast<std::size_t>(Control::Count);

/** @brief The most keys one control can have. */
inline constexpr std::size_t MaxKeysPerControl = 8;

/** @brief The keys bound to each control, indexed by Control. */
using Controls = std::array<std::vector<gfx::Key>, ControlCount>;

/**
 * @brief The default bindings, playable with either hand alone: arrows,
 * Right Ctrl (fire) and Right Shift (auto-fire) on the right; W A S D,
 * Space (fire) and F (auto-fire) on the left.
 * @return The default controls.
 **/
[[nodiscard]] Controls defaultControls();

/**
 * @brief Finds a key by its name, ignoring case: "Up", "Space", "Enter",
 * "Tab", "LeftShift", "RightCtrl", "LeftAlt", "A" to "Z", "0" to "9"...
 * @param name The name of the key.
 * @return The key, or nothing if no key has this name.
 **/
[[nodiscard]] std::optional<gfx::Key> keyFromName(std::string_view name);

/**
 * @brief The name of a key, as keyFromName() reads it.
 * @param key The key.
 * @return Its name, "?" for Key::Count.
 **/
[[nodiscard]] std::string keyName(gfx::Key key);

/**
 * @brief Finds a control by its name, ignoring case: "up", "down",
 * "left", "right", "fire", "autofire".
 * @param name The name of the control.
 * @return The control, or nothing if no control has this name.
 **/
[[nodiscard]] std::optional<Control> controlFromName(std::string_view name);

/**
 * @brief Reads a controls file. Each line is `control = key key...`
 * (`fire = Space Enter`) and replaces the keys of that control; empty
 * lines and lines starting with `#` are ignored. A bad line is skipped
 * and reported, the other lines still apply.
 * @param in The file to read.
 * @param controls The controls to change, usually defaultControls().
 * @return One message per line that could not be read, with its number.
 **/
std::vector<std::string> readControls(std::istream &in, Controls &controls);

} // namespace client

#endif // CLIENT_CONTROLS_HPP
