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
#include <fstream>
#include <iostream>
#include <istream>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace client {

/** @brief The controls file read if --controls is not given. */
inline constexpr const char *DefaultControlsPath = "controls.cfg";

/**
 * @enum Control
 * @brief What the player can do with the keyboard.
 * @param Up Moves the ship up.
 * @param Down Moves the ship down.
 * @param Left Moves the ship to the left.
 * @param Right Moves the ship to the right.
 * @param Fire Fires, or charges a shot while held.
 * @param AutoFire Turns auto-fire on or off: the ship fires without a key held.
 * @param Chat Opens the chat of the room.
 * @param Count The number of controls, not a control.
 **/
enum class Control : std::uint8_t {
    Up,
    Down,
    Left,
    Right,
    Fire,
    AutoFire,
    Chat,
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
 * Space (fire), F (auto-fire) and T (chat) on the left.
 * @return The default controls.
 **/
[[nodiscard]] Controls defaultControls();

/**
 * @brief The default gamepad buttons: D-pad or left stick to move, A (Cross)
 * or RT (R2) to fire, RB (R1) for auto-fire. The chat has none, as writing
 * needs a keyboard.
 * @return The default gamepad controls.
 **/
[[nodiscard]] Controls defaultPadControls();

/**
 * @struct PlayerControls
 * @brief The keys of each control on each device. Both work at the same time.
 * @param keyboard The keys of the keyboard.
 * @param gamepad The buttons of the gamepad.
 **/
struct PlayerControls {
    Controls keyboard = defaultControls();
    Controls gamepad = defaultPadControls();

    bool operator==(const PlayerControls &) const = default;
};

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
 * "left", "right", "fire", "autofire", "chat".
 * @param name The name of the control.
 * @return The control, or nothing if no control has this name.
 **/
[[nodiscard]] std::optional<Control> controlFromName(std::string_view name);

/**
 * @brief Reads a controls file. Each line is `control = key key...`
 * (`fire = Space Enter`) and replaces the keyboard keys of that control, or
 * `pad_control = button...` (`pad_fire = PadA`) for the gamepad. Empty lines
 * and lines starting with `#` are ignored. A bad line, such as a gamepad button
 * on a keyboard line, is skipped and reported; the other lines still apply.
 * @param in The file to read.
 * @param controls The controls to change, usually the default ones.
 * @return One message per line that could not be read, with its number.
 **/
std::vector<std::string> readControls(std::istream &in,
                                      PlayerControls &controls);

/**
 * @brief Writes a controls file that readControls() can read back: the
 * keyboard lines, then the gamepad lines.
 * @param out The file to write.
 * @param controls The controls.
 **/
void writeControls(std::ostream &out, const PlayerControls &controls);

/**
 * @brief Reads a controls file over the default controls and reports its bad
 * lines on stderr.
 * @param path The file given with --controls, or nothing to read
 * DefaultControlsPath from the current directory if it exists.
 * @return The controls.
 * @throws std::runtime_error If the file given with --controls cannot be
 * opened.
 **/
[[nodiscard]] PlayerControls
loadControlsFile(const std::optional<std::string> &path);

/**
 * @brief Replaces a controls file, and reports on stderr if it cannot.
 * @param path The file.
 * @param controls The controls.
 * @return Whether the file was written.
 **/
bool saveControlsFile(const std::string &path, const PlayerControls &controls);

/**
 * @brief Binds a key to one place of a control. If the control already has the
 * key at another place, the two places swap their keys, so that it never has
 * the same key twice.
 * @param controls The controls, updated.
 * @param control The control.
 * @param slot The place among its keys; past the last one, the key is added.
 * @param key The key.
 **/
void bindKey(Controls &controls, Control control, std::size_t slot,
             gfx::Key key);

/**
 * @brief Finds another control that uses a key.
 * @param controls The controls.
 * @param control The control to leave out.
 * @param key The key.
 * @return The first other control with this key, if any.
 **/
[[nodiscard]] std::optional<Control>
otherControlWith(const Controls &controls, Control control, gfx::Key key);

} // namespace client

#endif // CLIENT_CONTROLS_HPP
