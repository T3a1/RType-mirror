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
#include <cstddef>
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
 * @struct WindowSize
 * @brief A size of the window.
 * @param width Its width, in pixels.
 * @param height Its height, in pixels.
 **/
struct WindowSize {
    int width;
    int height;
};

/** @brief The sizes the player can choose. The menus were made for the first,
 * so none is smaller. */
inline constexpr std::array WindowSizes{
    WindowSize{.width = 1280, .height = 720},
    WindowSize{.width = 1600, .height = 900},
    WindowSize{.width = 1920, .height = 1080},
    WindowSize{.width = 2560, .height = 1440},
};

/** @brief The frame limits the player can choose, per second; 0 is no limit. */
inline constexpr std::array FrameLimits{15, 30, 60, 120, 0};

/**
 * @struct Settings
 * @brief What the player chooses in the menus, kept in settings.cfg.
 * @param musicVolume The volume of the music, from 0 to 1.
 * @param soundVolume The volume of the sounds, from 0 to 1.
 * @param autoFire Whether auto-fire is on when the game starts.
 * @param playerLabels Whether the name of each player is shown above the ship.
 * @param background Whether the scrolling background is drawn. Off, the game
 * is drawn on a still black screen, for players bothered by the motion.
 * @param reduceFlashing Whether flashing effects are calmed: destroyed ships
 * and enemies vanish without an explosion, and the charge glow is still.
 * @param fullscreen Whether the window covers the screen.
 * @param vsync Whether frames wait for the screen's refresh. It replaces the
 * frame limit.
 * @param windowSize The size of the window, as an index in WindowSizes.
 * @param frameLimit The frame limit, as an index in FrameLimits.
 **/
struct Settings {
    float musicVolume = 0.4F;
    float soundVolume = 1.0F;
    bool autoFire = true;
    bool playerLabels = true;
    bool background = true;
    bool reduceFlashing = false;
    bool fullscreen = false;
    bool vsync = true;
    std::size_t windowSize = 0;
    std::size_t frameLimit = FrameLimits.size() - 1;
};

/**
 * @brief Returns the text of a window size, in the file and the menus.
 * @param size The size.
 * @return Its text, such as "1280x720".
 **/
[[nodiscard]] std::string windowSizeName(WindowSize size);

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

/** @brief Every accessibility setting that is on or off: the file and the menu
 * read this table, so adding a line here is enough to have one more. */
inline constexpr std::array Toggles{
    ToggleDef{
        .key = "auto_fire", .label = "Auto-fire", .field = &Settings::autoFire},
    ToggleDef{.key = "player_labels",
              .label = "Player labels",
              .field = &Settings::playerLabels},
    ToggleDef{.key = "background",
              .label = "Background",
              .field = &Settings::background},
    ToggleDef{.key = "reduce_flashing",
              .label = "Reduce flashing",
              .field = &Settings::reduceFlashing},
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
