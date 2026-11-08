/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ShotStyles
*/

#ifndef SERVER_COMPONENTS_SHOTSTYLES_HPP
#define SERVER_COMPONENTS_SHOTSTYLES_HPP

#include "common/components/PowerUpKind.hpp"
#include "engine/Clock.hpp"

#include <array>
#include <cstdint>

namespace server {

// Shot styles: how a player's weapon turns a shot (Weapon::fired) into
// projectiles. Each style is a component applied by its own system
// (systems/ShotStyleSystems.hpp), so a player can have several at once:
// they stack. Every ship has a SpreadShot; power-ups add the others for a
// while (Boosts). See docs/WEAPONS.md to add a style.

/**
 * @struct SpreadShot
 * @brief Fires 2 * level - 1 shots at once, fanned out: a single shot at
 * level 1. Every ship has it; a Spread boost raises its level.
 * @param level Its level, from 1 to WeaponStats::maxLevel.
 **/
struct SpreadShot {
    std::uint8_t level = 1;
};

/**
 * @struct AimedShot
 * @brief Fires one more shot, aimed at the nearest enemy. Given by an
 * Aimed boost.
 **/
struct AimedShot {};

/**
 * @struct BurstShot
 * @brief Fires each shot again WeaponStats::burstShots times, right after
 * it, with every other style of the player. Given by a Burst boost.
 * @param pending The shots of the burst still to fire.
 * @param delay The time left before the next one.
 **/
struct BurstShot {
    std::uint8_t pending = 0;
    engine::Duration delay{0.0F};
};

/**
 * @struct Boosts
 * @brief The time left of each boost of a player, by PowerUpKind; 0 when
 * it does not have it. Every ship has it.
 * @param left The time left of each boost.
 **/
struct Boosts {
    std::array<engine::Duration, rtype::PowerUpKindCount> left{};

    /**
     * @brief Returns the time left of a boost.
     * @param kind The boost.
     * @return Its time left, 0 if the player does not have it.
     **/
    [[nodiscard]] engine::Duration &of(rtype::PowerUpKind kind)
    {
        return left.at(static_cast<std::size_t>(kind));
    }
};

} // namespace server

#endif // SERVER_COMPONENTS_SHOTSTYLES_HPP
