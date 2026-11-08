/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Controls
*/

#include "Controls.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <utility>

namespace client {

namespace {

using gfx::Key;

/** @brief The names of the keys that are not letters or digits. */
constexpr std::array<std::pair<std::string_view, Key>, 29> NamedKeys{{
    {"Up", Key::Up},
    {"Down", Key::Down},
    {"Left", Key::Left},
    {"Right", Key::Right},
    {"Space", Key::Space},
    {"Enter", Key::Enter},
    {"Tab", Key::Tab},
    {"LeftShift", Key::LeftShift},
    {"RightShift", Key::RightShift},
    {"LeftCtrl", Key::LeftControl},
    {"RightCtrl", Key::RightControl},
    {"LeftAlt", Key::LeftAlt},
    {"RightAlt", Key::RightAlt},
    {"PadUp", Key::PadUp},
    {"PadDown", Key::PadDown},
    {"PadLeft", Key::PadLeft},
    {"PadRight", Key::PadRight},
    {"PadStickUp", Key::PadStickUp},
    {"PadStickDown", Key::PadStickDown},
    {"PadStickLeft", Key::PadStickLeft},
    {"PadStickRight", Key::PadStickRight},
    {"PadA", Key::PadA},
    {"PadB", Key::PadB},
    {"PadLB", Key::PadLB},
    {"PadRB", Key::PadRB},
    {"PadLT", Key::PadLT},
    {"PadRT", Key::PadRT},
    {"PadStart", Key::PadStart},
    {"PadSelect", Key::PadSelect},
}};

/** @brief The name of each control in the file, in the order of Control. */
constexpr std::array<std::string_view, ControlCount> ControlNames{
    "up", "down", "left", "right", "fire", "autofire", "chat",
};

/** @brief The start of the name of a gamepad line in the file. */
constexpr std::string_view PadPrefix = "pad_";

/** @brief The number of letter keys, from Key::A. */
constexpr int LetterCount = 26;
/** @brief The number of digit keys, from Key::Num0. */
constexpr int DigitCount = 10;

/**
 * @brief Compares two names, ignoring case.
 * @param a The first name.
 * @param b The second name.
 * @return `true` if they are equal.
 **/
bool sameName(std::string_view a, std::string_view b)
{
    return std::ranges::equal(a, b, [](char x, char y) {
        return std::tolower(static_cast<unsigned char>(x)) ==
               std::tolower(static_cast<unsigned char>(y));
    });
}

/**
 * @brief Removes the blanks around a text.
 * @param text The text.
 * @return The text without its leading and trailing blanks.
 **/
std::string_view trim(std::string_view text)
{
    const auto first = text.find_first_not_of(" \t\r");
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = text.find_last_not_of(" \t\r");
    return text.substr(first, last - first + 1);
}

/**
 * @brief Reads one line of a controls file.
 * @param line The line, without its end of line.
 * @param controls The controls to change.
 * @return An error message, or nothing if the line was applied or empty.
 **/
std::optional<std::string> readLine(std::string_view line,
                                    PlayerControls &controls)
{
    line = trim(line);
    if (line.empty() || line.front() == '#') {
        return std::nullopt;
    }
    const auto equal = line.find('=');
    if (equal == std::string_view::npos) {
        return "expected \"control = key...\"";
    }
    const std::string_view name = trim(line.substr(0, equal));
    const bool pad = name.size() > PadPrefix.size() &&
                     sameName(name.substr(0, PadPrefix.size()), PadPrefix);
    const std::optional<Control> control =
        controlFromName(pad ? name.substr(PadPrefix.size()) : name);
    if (!control) {
        return "unknown control \"" + std::string{name} + "\"";
    }
    const std::string_view controlName =
        ControlNames.at(static_cast<std::size_t>(*control));
    std::vector<Key> keys;
    std::istringstream words{std::string{line.substr(equal + 1)}};
    for (std::string word; words >> word;) {
        const std::optional<Key> key = keyFromName(word);
        if (!key) {
            return "unknown key \"" + word + "\"";
        }
        if (gfx::isPadKey(*key) && !pad) {
            return word + " is a gamepad button: write it in " +
                   std::string{PadPrefix} + std::string{controlName};
        }
        if (!gfx::isPadKey(*key) && pad) {
            return word + " is a keyboard key: write it in " +
                   std::string{controlName};
        }
        if (std::ranges::find(keys, *key) == keys.end()) {
            keys.push_back(*key);
        }
    }
    if (keys.empty()) {
        return "no key for \"" + std::string{name} + "\"";
    }
    if (keys.size() > MaxKeysPerControl) {
        return "too many keys for \"" + std::string{name} + "\"";
    }
    Controls &device = pad ? controls.gamepad : controls.keyboard;
    device.at(static_cast<std::size_t>(*control)) = std::move(keys);
    return std::nullopt;
}

/**
 * @brief Writes the lines of one device in a controls file.
 * @param out The file to write.
 * @param controls The keys of the device.
 * @param prefix What starts the name of each line: empty for the keyboard,
 * PadPrefix for the gamepad.
 **/
void writeDevice(std::ostream &out, const Controls &controls,
                 std::string_view prefix)
{
    for (std::size_t i = 0; i < ControlCount; ++i) {
        if (controls.at(i).empty()) {
            continue;
        }
        out << prefix << ControlNames.at(i) << " =";
        for (const Key key : controls.at(i)) {
            out << ' ' << keyName(key);
        }
        out << '\n';
    }
}

} // namespace

Controls defaultControls()
{
    Controls controls;
    const auto set = [&controls](Control control, std::vector<Key> keys) {
        controls.at(static_cast<std::size_t>(control)) = std::move(keys);
    };
    set(Control::Up, {Key::Up, Key::W});
    set(Control::Down, {Key::Down, Key::S});
    set(Control::Left, {Key::Left, Key::A});
    set(Control::Right, {Key::Right, Key::D});
    set(Control::Fire, {Key::Space, Key::RightControl});
    set(Control::AutoFire, {Key::F, Key::RightShift});
    set(Control::Chat, {Key::T});
    return controls;
}

Controls defaultPadControls()
{
    Controls controls;
    const auto set = [&controls](Control control, std::vector<Key> keys) {
        controls.at(static_cast<std::size_t>(control)) = std::move(keys);
    };
    set(Control::Up, {Key::PadUp, Key::PadStickUp});
    set(Control::Down, {Key::PadDown, Key::PadStickDown});
    set(Control::Left, {Key::PadLeft, Key::PadStickLeft});
    set(Control::Right, {Key::PadRight, Key::PadStickRight});
    set(Control::Fire, {Key::PadA, Key::PadRT});
    set(Control::AutoFire, {Key::PadRB});
    return controls;
}

std::optional<Key> keyFromName(std::string_view name)
{
    for (const auto &[keyName, key] : NamedKeys) {
        if (sameName(name, keyName)) {
            return key;
        }
    }
    if (name.size() != 1) {
        return std::nullopt;
    }
    const auto character =
        static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
    if (character >= 'A' && character <= 'Z') {
        return static_cast<Key>(static_cast<int>(Key::A) + character - 'A');
    }
    if (character >= '0' && character <= '9') {
        return static_cast<Key>(static_cast<int>(Key::Num0) + character - '0');
    }
    return std::nullopt;
}

std::string keyName(Key key)
{
    for (const auto &[name, named] : NamedKeys) {
        if (named == key) {
            return std::string{name};
        }
    }
    const auto index = static_cast<int>(key);
    const int letter = index - static_cast<int>(Key::A);
    const int digit = index - static_cast<int>(Key::Num0);
    std::string name = "?";
    if (letter >= 0 && letter < LetterCount) {
        name = static_cast<char>('A' + letter);
    } else if (digit >= 0 && digit < DigitCount) {
        name = static_cast<char>('0' + digit);
    }
    return name;
}

std::optional<Control> controlFromName(std::string_view name)
{
    for (std::size_t i = 0; i < ControlNames.size(); ++i) {
        if (sameName(name, ControlNames.at(i))) {
            return static_cast<Control>(i);
        }
    }
    return std::nullopt;
}

std::vector<std::string> readControls(std::istream &in,
                                      PlayerControls &controls)
{
    std::vector<std::string> errors;
    int number = 0;
    for (std::string line; std::getline(in, line);) {
        ++number;
        if (const auto error = readLine(line, controls)) {
            errors.push_back("line " + std::to_string(number) + ": " + *error);
        }
    }
    return errors;
}

void writeControls(std::ostream &out, const PlayerControls &controls)
{
    out << "# R-Type controls\n";
    writeDevice(out, controls.keyboard, "");
    writeDevice(out, controls.gamepad, PadPrefix);
}

PlayerControls loadControlsFile(const std::optional<std::string> &path)
{
    PlayerControls controls;
    const std::string file = path.value_or(DefaultControlsPath);
    std::ifstream in(file);
    if (!in) {
        if (path) {
            throw std::runtime_error("cannot open " + file);
        }
        return controls;
    }
    for (const std::string &error : readControls(in, controls)) {
        std::cerr << "r-type_client: " << file << ": " << error << '\n';
    }
    return controls;
}

bool saveControlsFile(const std::string &path, const PlayerControls &controls)
{
    std::ofstream out(path);
    writeControls(out, controls);
    out.flush();
    if (!out) {
        std::cerr << "r-type_client: cannot write " << path << '\n';
        return false;
    }
    return true;
}

void bindKey(Controls &controls, Control control, std::size_t slot, Key key)
{
    std::vector<Key> &keys = controls.at(static_cast<std::size_t>(control));
    const auto existing = std::ranges::find(keys, key);
    if (slot >= keys.size()) {
        if (existing == keys.end()) {
            keys.push_back(key);
        }
        return;
    }
    if (existing != keys.end()) {
        *existing = keys.at(slot);
    }
    keys.at(slot) = key;
}

std::optional<Control> otherControlWith(const Controls &controls,
                                        Control control, Key key)
{
    for (std::size_t i = 0; i < ControlCount; ++i) {
        const auto other = static_cast<Control>(i);
        if (other != control &&
            std::ranges::find(controls.at(i), key) != controls.at(i).end()) {
            return other;
        }
    }
    return std::nullopt;
}

} // namespace client
