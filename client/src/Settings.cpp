/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Settings
*/

#include "Settings.hpp"

namespace client {

namespace {

/** @brief The name of the music volume in the settings file. */
constexpr std::string_view MusicVolumeKey = "music_volume";
/** @brief The name of the sound volume in the settings file. */
constexpr std::string_view SoundVolumeKey = "sound_volume";
/** @brief The name of the fullscreen option in the settings file. */
constexpr std::string_view FullscreenKey = "fullscreen";
/** @brief The name of the window size in the settings file. */
constexpr std::string_view WindowSizeKey = "window_size";
/** @brief The name of the V-Sync option in the settings file. */
constexpr std::string_view VsyncKey = "vsync";
/** @brief The name of the frame limit in the settings file. */
constexpr std::string_view FrameLimitKey = "frame_limit";
/** @brief The characters ignored around a name or a value. */
constexpr std::string_view Blanks = " \t\r";

/**
 * @brief Removes the blanks around a text.
 * @param text The text.
 * @return The text without them.
 **/
std::string_view trim(std::string_view text)
{
    const std::size_t first = text.find_first_not_of(Blanks);
    if (first == std::string_view::npos) {
        return {};
    }
    return text.substr(first, text.find_last_not_of(Blanks) - first + 1);
}

/**
 * @brief Reads a volume.
 * @param text The value.
 * @param volume The volume, unchanged if the value is bad.
 * @return The error, if any.
 **/
std::optional<std::string> readVolume(std::string_view text, float &volume)
{
    float value = 0.0F;
    const char *end = text.data() + text.size();
    const auto [parsed, error] = std::from_chars(text.data(), end, value);
    // Written this way, the test is also false for "nan".
    const bool inRange = value >= 0.0F && value <= 1.0F;
    if (error != std::errc{} || parsed != end || !inRange) {
        return "a volume goes from 0 to 1, not \"" + std::string{text} + "\"";
    }
    volume = value;
    return std::nullopt;
}

/**
 * @brief Reads a setting that is on or off.
 * @param text The value.
 * @param toggle The setting, unchanged if the value is bad.
 * @return The error, if any.
 **/
std::optional<std::string> readToggle(std::string_view text, bool &toggle)
{
    if (text != "on" && text != "off") {
        return "expected on or off, not \"" + std::string{text} + "\"";
    }
    toggle = text == "on";
    return std::nullopt;
}

/**
 * @brief Reads a window size.
 * @param text The value.
 * @param index The index of the size, unchanged if the value is bad.
 * @return The error, if any.
 **/
std::optional<std::string> readWindowSize(std::string_view text,
                                          std::size_t &index)
{
    for (std::size_t i = 0; i < WindowSizes.size(); ++i) {
        if (text == windowSizeName(WindowSizes.at(i))) {
            index = i;
            return std::nullopt;
        }
    }
    return "unknown window size \"" + std::string{text} + "\"";
}

/**
 * @brief Reads a frame limit.
 * @param text The value.
 * @param index The index of the limit, unchanged if the value is bad.
 * @return The error, if any.
 **/
std::optional<std::string> readFrameLimit(std::string_view text,
                                          std::size_t &index)
{
    for (std::size_t i = 0; i < FrameLimits.size(); ++i) {
        if (text == std::to_string(FrameLimits.at(i))) {
            index = i;
            return std::nullopt;
        }
    }
    return "unknown frame limit \"" + std::string{text} + "\"";
}

/**
 * @brief Reads one line of a settings file.
 * @param line The line.
 * @param settings The settings to change.
 * @return The error, if any.
 **/
std::optional<std::string> readLine(std::string_view line, Settings &settings)
{
    const std::string_view content = trim(line);
    if (content.empty() || content.front() == '#') {
        return std::nullopt;
    }
    const std::size_t equal = content.find('=');
    if (equal == std::string_view::npos) {
        return "expected name = value";
    }
    const std::string_view key = trim(content.substr(0, equal));
    const std::string_view value = trim(content.substr(equal + 1));
    if (key == MusicVolumeKey) {
        return readVolume(value, settings.musicVolume);
    }
    if (key == SoundVolumeKey) {
        return readVolume(value, settings.soundVolume);
    }
    if (key == FullscreenKey) {
        return readToggle(value, settings.fullscreen);
    }
    if (key == VsyncKey) {
        return readToggle(value, settings.vsync);
    }
    if (key == WindowSizeKey) {
        return readWindowSize(value, settings.windowSize);
    }
    if (key == FrameLimitKey) {
        return readFrameLimit(value, settings.frameLimit);
    }
    for (const ToggleDef &toggle : Toggles) {
        if (key == toggle.key) {
            return readToggle(value, settings.*toggle.field);
        }
    }
    return "unknown setting \"" + std::string{key} + "\"";
}

} // namespace

std::vector<std::string> readSettings(std::istream &in, Settings &settings)
{
    std::vector<std::string> errors;
    int number = 0;
    for (std::string line; std::getline(in, line);) {
        ++number;
        if (const auto error = readLine(line, settings)) {
            errors.push_back("line " + std::to_string(number) + ": " + *error);
        }
    }
    return errors;
}

void writeSettings(std::ostream &out, const Settings &settings)
{
    out << "# R-Type settings\n"
        << MusicVolumeKey << " = " << settings.musicVolume << '\n'
        << SoundVolumeKey << " = " << settings.soundVolume << '\n'
        << FullscreenKey << " = " << (settings.fullscreen ? "on" : "off")
        << '\n'
        << WindowSizeKey << " = "
        << windowSizeName(WindowSizes.at(settings.windowSize)) << '\n'
        << VsyncKey << " = " << (settings.vsync ? "on" : "off") << '\n'
        << FrameLimitKey << " = " << FrameLimits.at(settings.frameLimit)
        << '\n';
    for (const ToggleDef &toggle : Toggles) {
        out << toggle.key << " = " << (settings.*toggle.field ? "on" : "off")
            << '\n';
    }
}

std::string windowSizeName(WindowSize size)
{
    return std::to_string(size.width) + "x" + std::to_string(size.height);
}

Settings loadSettingsFile(const std::string &path)
{
    Settings settings;
    std::ifstream in(path);
    if (!in) {
        return settings;
    }
    for (const std::string &error : readSettings(in, settings)) {
        std::cerr << "r-type_client: " << path << ": " << error << '\n';
    }
    return settings;
}

bool saveSettingsFile(const std::string &path, const Settings &settings)
{
    std::ofstream out(path);
    writeSettings(out, settings);
    out.flush();
    if (!out) {
        std::cerr << "r-type_client: cannot write " << path << '\n';
        return false;
    }
    return true;
}

} // namespace client
