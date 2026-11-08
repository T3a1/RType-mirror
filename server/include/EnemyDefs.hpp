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
inline constexpr std::array<EnemyDef, 16> ENEMY_DEFS = {
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
        {
            .speed = 160.0F,
            .health = 120,
            .size = {.x = 76.0F, .y = 70.0F},
            .entityType = rtype::EntityType::EnemySpinner,
            .score = 180,
            .fireInterval = engine::Duration{1.6F},
            .pattern = {.kind = rtype::MovementKind::Sine,
                        .amplitude = 160.0F,
                        .frequency = 0.4F},
        },
        {
            .speed = 300.0F,
            .health = 60,
            .size = {.x = 76.0F, .y = 66.0F},
            .entityType = rtype::EntityType::EnemyBubble,
            .score = 80,
            .pattern = {.kind = rtype::MovementKind::Sine,
                        .amplitude = 220.0F,
                        .frequency = 0.8F},
        },
        {
            .speed = 110.0F,
            .health = 260,
            .size = {.x = 90.0F, .y = 84.0F},
            .entityType = rtype::EntityType::EnemySpore,
            .score = 250,
            .pattern = {.kind = rtype::MovementKind::Zigzag,
                        .amplitude = 200.0F,
                        .frequency = 0.3F},
            .dropsPowerUp = true,
        },
        {
            .speed = 220.0F,
            .health = 90,
            .size = {.x = 76.0F, .y = 76.0F},
            .entityType = rtype::EntityType::EnemyJelly,
            .score = 120,
            .pattern = {.kind = rtype::MovementKind::Dive,
                        .diveDelay = engine::Duration{1.5F},
                        .diveSpeed = 700.0F},
        },
        {
            .speed = 130.0F,
            .health = 180,
            .size = {.x = 90.0F, .y = 80.0F},
            .entityType = rtype::EntityType::EnemyCrab,
            .score = 260,
            .fireInterval = engine::Duration{1.2F},
            .dropsPowerUp = true,
        },
        {
            // The walker: on the ground, walks left and right.
            .speed = 140.0F,
            .health = 3000,
            .size = {.x = 180.0F, .y = 200.0F},
            .entityType = rtype::EntityType::BossWalker,
            .score = 7000,
            .dropsPowerUp = true,
        },
        {
            // The twin: this half on the ground, the other on the ceiling
            // (a BossPart), neither moving once in place.
            .speed = 120.0F,
            .health = 3600,
            .size = {.x = 200.0F, .y = 210.0F},
            .entityType = rtype::EntityType::BossTwin,
            .score = 8000,
            .dropsPowerUp = true,
        },
        {
            // The behemoth's body; its head and tail are BossParts.
            .speed = 90.0F,
            .health = 4500,
            .size = {.x = 220.0F, .y = 400.0F},
            .entityType = rtype::EntityType::BossBehemoth,
            .score = 10000,
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
