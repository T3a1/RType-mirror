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
 * @param dropsPowerUp Whether it leaves a power-up when it is destroyed.
 **/
struct EnemyDef {
    float speed = 0.0F;
    int health = 0;
    engine::Vec2f size{};
    rtype::EntityType entityType = rtype::EntityType::EnemyBasic;
    std::uint32_t score = 0;
    engine::Duration fireInterval{0.0F};
    rtype::MovementPattern pattern{};
    bool dropsPowerUp = false;
};

/**
 * @brief The default stats of the built-in enemies, in the order of
 * rtype::EnemyType, used when server/enemies.json does not change them.
 *The colliders match the sizes the clients draw (client/src/SpriteSheets.cpp).
 **/
inline constexpr std::array<EnemyDef, 8> ENEMY_DEFS = {
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
            .dropsPowerUp = true,
        },
        {
            .speed = 70.0F,
            .health = 500,
            .size = {.x = 126.0F, .y = 96.0F},
            .entityType = rtype::EntityType::EnemyLarge,
            .score = 500,
            .fireInterval = engine::Duration{1.0F},
            .dropsPowerUp = true,
        },
        {
            // Moved and armed by bossSystem(), not by its pattern or Weapon.
            .speed = 160.0F,
            .health = rtype::BossMaxHealth,
            .size = {.x = 380.0F, .y = 230.0F},
            .entityType = rtype::EntityType::Boss,
            .score = 5000,
            .dropsPowerUp = true,
        },
        {
            // The hive: slower, larger, on the ground; armed by bossSystem().
            .speed = 110.0F,
            .health = rtype::BossMaxHealth,
            .size = {.x = 400.0F, .y = 230.0F},
            .entityType = rtype::EntityType::BossHive,
            .score = 5000,
            .dropsPowerUp = true,
        },
    },
};

/**
 * @brief Returns the stats of a built-in enemy type in the catalog of the
 * server (see EnemyCatalog.hpp): those of ENEMY_DEFS, or of
 * server/enemies.json once it is loaded.
 * @param type The type of the enemy.
 * @return The definition of that type.
 **/
[[nodiscard]] const EnemyDef &enemyDef(rtype::EnemyType type);

/**
 * @brief Returns the stats of an enemy: those of its kind in the catalog of
 * the server, or of its type when its kind is a built-in one.
 * @param enemy The Enemy component of the entity.
 * @return Its definition.
 **/
[[nodiscard]] const EnemyDef &enemyDef(const rtype::Enemy &enemy);

} // namespace server

#endif // SERVER_ENEMYDEFS_HPP
