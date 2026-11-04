/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EnemyDefs
*/

#ifndef SERVER_ENEMYDEFS_HPP
#define SERVER_ENEMYDEFS_HPP

#include "common/components/Enemy.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/MovementPattern.hpp"
#include "engine/Clock.hpp"
#include "engine/Vec2.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace server {

/**
 * @struct EnemyDef
 * @brief Stats shared by every enemy of one type.
 * @param speed Speed towards the left, in playfield units per second.
 * @param health Health points the enemy starts with.
 * @param size Width and height of its collider, in playfield units.
 * @param entityType What the clients draw for it (Basic, Wave, Shooter...).
 * @param score The points earned by the player who destroys it.
 * @param fireInterval Time between two shots, in seconds; 0 if it does not
 * shoot.
 * @param pattern The path it follows, unless its spawn request overrides it.
 **/
struct EnemyDef {
    float speed = 0.0F;
    int health = 0;
    engine::Vec2f size{};
    rtype::EntityType entityType = rtype::EntityType::EnemyBasic;
    std::uint32_t score = 0;
    engine::Duration fireInterval{0.0F};
    rtype::MovementPattern pattern{};
};

/**
 * An array of all enemy definitions, in the order of rtype::EnemyType. The
 * colliders match the sizes the clients draw (client/src/SpriteSheets.cpp).
 **/
inline constexpr std::array<EnemyDef, 6> ENEMY_DEFS = {
    {
        {
            .speed = 200.0F,
            .health = 100,
            .size = {.x = 63.0F, .y = 72.0F},
            .entityType = rtype::EntityType::EnemyBasic,
            .score = 100,
        },
        {
            .speed = 250.0F,
            .health = 50,
            .size = {.x = 78.0F, .y = 65.0F},
            .entityType = rtype::EntityType::EnemyWave,
            .score = 50,
            .pattern = {.kind = rtype::MovementKind::Sine,
                        .amplitude = 120.0F,
                        .frequency = 0.5F},
        },
        {
            .speed = 100.0F,
            .health = 100,
            .size = {.x = 80.0F, .y = 60.0F},
            .entityType = rtype::EntityType::EnemyShooter,
            .score = 150,
            .fireInterval = engine::Duration{1.5F},
        },
        {
            .speed = 300.0F,
            .health = 50,
            .size = {.x = 51.0F, .y = 51.0F},
            .entityType = rtype::EntityType::EnemySmall,
            .score = 25,
            .pattern = {.kind = rtype::MovementKind::Sine,
                        .amplitude = 140.0F,
                        .frequency = 0.7F},
        },
        {
            .speed = 150.0F,
            .health = 150,
            .size = {.x = 96.0F, .y = 63.0F},
            .entityType = rtype::EntityType::EnemyMedium,
            .score = 200,
            .fireInterval = engine::Duration{2.0F},
            .pattern = {.kind = rtype::MovementKind::Zigzag,
                        .amplitude = 150.0F,
                        .frequency = 0.35F},
        },
        {
            .speed = 70.0F,
            .health = 500,
            .size = {.x = 126.0F, .y = 96.0F},
            .entityType = rtype::EntityType::EnemyLarge,
            .score = 500,
            .fireInterval = engine::Duration{1.0F},
        },
    },
};

/**
 * @brief Returns the stats of an enemy type from ENEMY_DEFS.
 * @param type The type of the enemy.
 * @return The definition of that type.
 **/
constexpr const EnemyDef &enemyDef(rtype::EnemyType type)
{
    return ENEMY_DEFS.at(static_cast<std::size_t>(type));
}

} // namespace server

#endif // SERVER_ENEMYDEFS_HPP
