/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Formations
*/

#ifndef SERVER_FORMATIONS_HPP
#define SERVER_FORMATIONS_HPP

#include "EnemyCatalog.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/MovementPattern.hpp"
#include "common/components/Spawner.hpp"
#include "components/SpawnQueue.hpp"
#include "engine/Clock.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

namespace server {

/**
 * @struct Formation
 * @brief A group of enemies of one type that arrives together.
 * @param type The type of the enemies.
 * @param count How many enemies it has.
 * @param spacing The time between two enemies spawning: same height and a
 * spacing make a snake, a height step and no spacing make a column.
 * @param stepY The vertical distance between two enemies, in playfield units.
 * @param fromCenter Whether the middle enemy spawns first and the others after
 * it, by distance to it (a V), instead of one end first.
 * @param pattern The path of the enemies, or nothing for the one of their type.
 * @param kind Their kind in the enemy catalog, which replaces type for their
 * stats; nothing for the built-in kind of type.
 **/
struct Formation {
    rtype::EnemyType type = rtype::EnemyType::Basic;
    std::size_t count = 1;
    engine::Duration spacing{0.0F};
    float stepY = 0.0F;
    bool fromCenter = false;
    std::optional<rtype::MovementPattern> pattern;
    std::optional<EnemyKindId> kind;
};

/**
 * @brief Every formation the server can send, all equally likely.
 **/
inline constexpr std::array<Formation, 7> FORMATIONS = {
    {
        // Snake of small enemies.
        {.type = rtype::EnemyType::Small,
         .count = 6,
         .spacing = engine::Duration{0.22F}},
        // Snake of waving enemies.
        {.type = rtype::EnemyType::Wave,
         .count = 4,
         .spacing = engine::Duration{0.35F}},
        // V of basic enemies.
        {.type = rtype::EnemyType::Basic,
         .count = 5,
         .spacing = engine::Duration{0.3F},
         .stepY = 90.0F,
         .fromCenter = true},
        // Staggered basic enemies diving at the players.
        {.type = rtype::EnemyType::Basic,
         .count = 3,
         .spacing = engine::Duration{0.5F},
         .stepY = 140.0F,
         .pattern = rtype::MovementPattern{.kind = rtype::MovementKind::Dive,
                                           .diveDelay = engine::Duration{2.0F},
                                           .diveSpeed = 650.0F}},
        // Column of zigzagging shooters.
        {.type = rtype::EnemyType::Medium, .count = 2, .stepY = 330.0F},
        // Pair of slow shooters.
        {.type = rtype::EnemyType::Shooter, .count = 2, .stepY = 300.0F},
        // One big enemy.
        {.type = rtype::EnemyType::Large},
    },
};

/**
 * @brief Returns how far a formation reaches above and below its center,
 * paths included.
 * @param formation The formation.
 * @return The distance, in playfield units.
 **/
[[nodiscard]] float formationExtent(const Formation &formation);

/**
 * @brief Draws a height for the center of a formation that keeps it inside
 * the playfield.
 * @param formation The formation.
 * @param spawner The spawner whose random generator is used.
 * @return The height, in playfield units.
 **/
[[nodiscard]] float randomCenterY(const Formation &formation,
                                  rtype::Spawner &spawner);

/**
 * @brief Builds the enemies of a formation, just past the right edge of the
 * playfield.
 * @param formation The formation.
 * @param centerY The height of its center, in playfield units.
 * @return One pending spawn per enemy.
 **/
[[nodiscard]] std::vector<PendingSpawn>
formationSpawns(const Formation &formation, float centerY);

} // namespace server

#endif // SERVER_FORMATIONS_HPP
