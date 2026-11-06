/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PowerUpKind
*/

#ifndef COMMON_COMPONENTS_POWERUPKIND_HPP
#define COMMON_COMPONENTS_POWERUPKIND_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace rtype {

/**
 * @enum PowerUpKind
 * @brief The boost a power-up gives for a while. Sent in snapshots: in the
 * flags of a power-up and in the boost timers of a player. New values go at
 * the end, before Count.
 * @param Spread The shots fan out: 3 shots, then 5 if taken again.
 * @param Aimed One more shot, aimed at the nearest enemy.
 * @param Burst Each shot is fired again twice, right after it.
 * @param Count The number of kinds, not a kind.
 **/
enum class PowerUpKind : std::uint8_t {
    Spread,
    Aimed,
    Burst,
    Count,
};

/** @brief The number of power-up kinds. */
inline constexpr std::size_t PowerUpKindCount =
    static_cast<std::size_t>(PowerUpKind::Count);

/** @brief Every power-up kind, in order. */
inline constexpr std::array<PowerUpKind, PowerUpKindCount> PowerUpKinds{
    PowerUpKind::Spread, PowerUpKind::Aimed, PowerUpKind::Burst};

/**
 * @brief Returns the name of a power-up kind, as shown to the players.
 * @param kind The kind.
 * @return Its name.
 **/
constexpr std::string_view powerUpName(PowerUpKind kind)
{
    switch (kind) {
    case PowerUpKind::Spread:
        return "Spread";
    case PowerUpKind::Aimed:
        return "Aimed";
    case PowerUpKind::Burst:
        return "Burst";
    case PowerUpKind::Count:
        break;
    }
    return "?";
}

} // namespace rtype

#endif // COMMON_COMPONENTS_POWERUPKIND_HPP
