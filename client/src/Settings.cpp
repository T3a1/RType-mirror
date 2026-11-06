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
        << SoundVolumeKey << " = " << settings.soundVolume << '\n';
    for (const ToggleDef &toggle : Toggles) {
        out << toggle.key << " = " << (settings.*toggle.field ? "on" : "off")
            << '\n';
    }
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
