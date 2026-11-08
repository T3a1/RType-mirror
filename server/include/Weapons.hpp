/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Weapons
*/

#ifndef SERVER_WEAPONS_HPP
#define SERVER_WEAPONS_HPP

#include "common/components/PowerUpKind.hpp"
#include "engine/Clock.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace server {

/** @brief The highest weapon level a power-up can reach. */
inline constexpr std::uint8_t MaxWeaponLevel = 5;

/**
 * @struct WeaponStats
 * @brief How the players' weapons and the enemies' shots behave, as read
 * from weapons.json (see docs/WEAPONS.md). The defaults are used for a
 * missing key or file.
 * @param fireInterval The time between two shots of a player.
 * @param shotSpeed The speed of a player's shot, in playfield units per
 * second.
 * @param shotDamage The damage of a player's shot.
 * @param enemyShotSpeed The speed of an enemy's shot.
 * @param chargeTime How long the fire key must be held for a charged shot.
 * @param beamSpeed The speed of a charged shot.
 * @param beamDamage The damage of a charged shot, dealt once to every
 * enemy it goes through.
 * @param spreadAngle The angle between two shots of a spread, in radians.
 * @param maxLevel The highest spread level: level N fires 2N - 1 shots.
 * @param boostDuration How long each boost of a power-up lasts, by
 * PowerUpKind.
 * @param burstShots How many times a Burst boost fires each shot again.
 * @param burstDelay The time between two shots of a burst.
 **/
struct WeaponStats {
    engine::Duration fireInterval{0.08F};
    float shotSpeed = 1400.0F;
    int shotDamage = 50;
    float enemyShotSpeed = 450.0F;
    engine::Duration chargeTime{1.0F};
    float beamSpeed = 1800.0F;
    int beamDamage = 300;
    float spreadAngle = 0.12F;
    std::uint8_t maxLevel = 3;
    std::array<engine::Duration, rtype::PowerUpKindCount> boostDuration{
        engine::Duration{30.0F}, engine::Duration{45.0F},
        engine::Duration{30.0F}};
    std::uint8_t burstShots = 2;
    engine::Duration burstDelay{0.08F};

    /**
     * @brief Returns how long a boost lasts.
     * @param kind The boost.
     * @return Its duration.
     **/
    [[nodiscard]] engine::Duration durationOf(rtype::PowerUpKind kind) const
    {
        return boostDuration.at(static_cast<std::size_t>(kind));
    }
};

/**
 * @brief Reads the weapon stats from the text of a JSON file. Every key is
 * optional; unknown keys and out-of-range values are errors.
 * @param text The content of the file.
 * @param error Set to what is wrong when the text cannot be read.
 * @return The stats, or nothing if the text is not valid.
 **/
[[nodiscard]] std::optional<WeaponStats> parseWeapons(std::string_view text,
                                                      std::string &error);

/**
 * @brief Reads the weapon stats from a file; never throws.
 * @param file The weapons.json file.
 * @param error Set to what is wrong when the defaults are used instead.
 * @return The stats of the file, or the defaults if it is missing, too
 * large or invalid.
 **/
[[nodiscard]] WeaponStats loadWeapons(const std::filesystem::path &file,
                                      std::string &error);

} // namespace server

#endif // SERVER_WEAPONS_HPP
