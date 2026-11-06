/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Weapons
*/

#include "Weapons.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <type_traits>

namespace server {

namespace {

/** @brief The JSON library's document type. */
using Json = nlohmann::json;

/** @brief Weapon files larger than this are not read, in bytes. */
constexpr std::uintmax_t MaxWeaponsFileSize = std::uintmax_t{64} * 1024U;

/** @brief What is wrong in a weapons file, caught by parseWeapons(). */
class WeaponsError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief Overwrites a value with a number of the file, if it is there.
 * @param root The JSON object.
 * @param key The key of the number.
 * @param min The smallest value allowed.
 * @param max The largest value allowed.
 * @param value The value to overwrite.
 * @throws WeaponsError If it is not a number or out of range.
 **/
template <typename T>
void read(const Json &root, const char *key, double min, double max, T &value)
{
    const auto it = root.find(key);
    if (it == root.end()) {
        return;
    }
    const bool whole = std::is_integral_v<T>;
    if (!it->is_number() || (whole && !it->is_number_integer())) {
        throw WeaponsError(std::string{key} + ": must be a " +
                           (whole ? "whole number" : "number"));
    }
    const auto number = it->get<double>();
    if (!std::isfinite(number) || number < min || number > max) {
        std::ostringstream message;
        message << key << ": must be from " << min << " to " << max;
        throw WeaponsError(message.str());
    }
    value = static_cast<T>(number);
}

/**
 * @brief Overwrites a duration with a number of seconds of the file, if it is
 * there.
 * @param root The object of the file.
 * @param key The name of the value.
 * @param min The lowest value allowed, in seconds.
 * @param max The highest value allowed, in seconds.
 * @param value The duration to overwrite.
 * @throws WeaponsError If the value is not a number or is out of range.
 **/
void readSeconds(const Json &root, const char *key, double min, double max,
                 engine::Duration &value)
{
    float seconds = value.count();
    read(root, key, min, max, seconds);
    value = engine::Duration{seconds};
}

} // namespace

std::optional<WeaponStats> parseWeapons(std::string_view text,
                                        std::string &error)
{
    try {
        const Json root = Json::parse(text);
        if (!root.is_object()) {
            throw WeaponsError("must be an object");
        }
        for (const auto &item : root.items()) {
            static constexpr std::array<std::string_view, 14> Keys{
                "fireInterval",   "shotSpeed",     "shotDamage",
                "enemyShotSpeed", "chargeTime",    "beamSpeed",
                "beamDamage",     "spreadAngle",   "maxLevel",
                "spreadDuration", "aimedDuration", "burstDuration",
                "burstShots",     "burstDelay"};
            if (std::ranges::find(Keys, item.key()) == Keys.end()) {
                throw WeaponsError("unknown key \"" + item.key() + "\"");
            }
        }
        WeaponStats stats;
        readSeconds(root, "fireInterval", 0.05, 5.0, stats.fireInterval);
        read(root, "shotSpeed", 100.0, 5000.0, stats.shotSpeed);
        read(root, "shotDamage", 1.0, 10000.0, stats.shotDamage);
        read(root, "enemyShotSpeed", 100.0, 5000.0, stats.enemyShotSpeed);
        readSeconds(root, "chargeTime", 0.1, 10.0, stats.chargeTime);
        read(root, "beamSpeed", 100.0, 5000.0, stats.beamSpeed);
        read(root, "beamDamage", 1.0, 10000.0, stats.beamDamage);
        read(root, "spreadAngle", 0.0, 1.0, stats.spreadAngle);
        read(root, "maxLevel", 1.0, double{MaxWeaponLevel}, stats.maxLevel);
        // In PowerUpKind order, up to 255 s: the snapshots send the
        // seconds left in one byte.
        readSeconds(root, "spreadDuration", 1.0, 255.0,
                    stats.boostDuration.at(0));
        readSeconds(root, "aimedDuration", 1.0, 255.0,
                    stats.boostDuration.at(1));
        readSeconds(root, "burstDuration", 1.0, 255.0,
                    stats.boostDuration.at(2));
        read(root, "burstShots", 1.0, 5.0, stats.burstShots);
        readSeconds(root, "burstDelay", 0.02, 1.0, stats.burstDelay);
        return stats;
    } catch (const Json::exception &exception) {
        error = exception.what();
    } catch (const WeaponsError &exception) {
        error = exception.what();
    }
    return std::nullopt;
}

WeaponStats loadWeapons(const std::filesystem::path &file, std::string &error)
{
    std::error_code code;
    const std::uintmax_t size = std::filesystem::file_size(file, code);
    if (code || size > MaxWeaponsFileSize) {
        error = file.string() + ": missing or larger than " +
                std::to_string(MaxWeaponsFileSize) + " bytes";
        return {};
    }
    std::ifstream in(file, std::ios::binary);
    const std::string text{std::istreambuf_iterator<char>(in),
                           std::istreambuf_iterator<char>()};
    std::string message;
    if (auto stats = parseWeapons(text, message)) {
        return *stats;
    }
    error = file.filename().string();
    error.append(": ").append(message);
    return {};
}

} // namespace server
