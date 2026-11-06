/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Settings
*/

#ifndef CLIENT_SETTINGS_HPP
#define CLIENT_SETTINGS_HPP

#include <array>
#include <charconv>
#include <fstream>
#include <iostream>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace client {

/** @brief The settings file, in the current directory. */
inline constexpr const char *SettingsPath = "settings.cfg";

/**
 * @struct Settings
 * @brief What the player chooses in the menus, kept in settings.cfg.
 * @param musicVolume The volume of the music, from 0 to 1.
 * @param soundVolume The volume of the sounds, from 0 to 1.
 * @param autoFire Whether auto-fire is on when the game starts.
 * @param playerLabels Whether the name of each player is shown above the ship.
 **/
struct Settings {
    float musicVolume = 0.4F;
    float soundVolume = 1.0F;
    bool autoFire = false;
    bool playerLabels = true;
};

/**
 * @struct ToggleDef
 * @brief A setting that is on or off.
 * @param key Its name in the settings file.
 * @param label Its text in the menus.
 * @param field The value it changes.
 **/
struct ToggleDef {
    const char *key;
    const char *label;
    bool Settings::*field;
};

/** @brief Every setting that is on or off: the file and the menus read this
 * table, so adding a line here is enough to have one more. */
inline constexpr std::array Toggles{
    ToggleDef{
        .key = "auto_fire", .label = "Auto-fire", .field = &Settings::autoFire},
    ToggleDef{.key = "player_labels",
              .label = "Player labels",
              .field = &Settings::playerLabels},
};

/**
 * @brief Reads a settings file. Each line is `name = value` (`music_volume =
 * 0.4`, `auto_fire = on`); empty lines and lines starting with `#` are ignored.
 * A bad line is skipped and reported, and its setting keeps its value.
 * @param in The file to read.
 * @param settings The settings to change, usually the default ones.
 * @return One message per line that could not be read, with its number.
 **/
std::vector<std::string> readSettings(std::istream &in, Settings &settings);

/**
 * @brief Writes a settings file that readSettings() can read back.
 * @param out The file to write.
 * @param settings The settings.
 **/
void writeSettings(std::ostream &out, const Settings &settings);

/**
 * @brief Reads a settings file over the default settings, if it exists, and
 * reports its bad lines on stderr.
 * @param path The file.
 * @return The settings.
 **/
[[nodiscard]] Settings loadSettingsFile(const std::string &path);

/**
 * @brief Replaces a settings file, and reports on stderr if it cannot.
 * @param path The file.
 * @param settings The settings.
 * @return Whether the file was written.
 **/
bool saveSettingsFile(const std::string &path, const Settings &settings);

} // namespace client

#endif // CLIENT_SETTINGS_HPP
