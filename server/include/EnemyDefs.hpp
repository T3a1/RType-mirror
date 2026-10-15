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
#include "engine/Vec2.hpp"

#include <array>
#include <cstddef>

namespace server {

/**
 * Stats shared by every enemy of one type.
 * @param speed The speed of the enemy.
 * @param health The health of the enemy.
 * @param size The size of the enemy.
 * @param entityType The entity type of the enemy (Basic, Wave, Shooter...).
 **/
struct EnemyDef {
    float speed = 0.0F;
    int health = 0;
    engine::Vec2f size{};
    rtype::EntityType entityType = rtype::EntityType::EnemyBasic;
};

/**
 * An array of all enemy definitions.
 **/
inline constexpr std::array<EnemyDef, 3> ENEMY_DEFS = {{
    {.speed = 200.0F,
     .health = 100,
     .size = {.x = 64.0F, .y = 64.0F},
     .entityType = rtype::EntityType::EnemyBasic},
    {.speed = 250.0F,
     .health = 50,
     .size = {.x = 64.0F, .y = 64.0F},
     .entityType = rtype::EntityType::EnemyWave},
    {.speed = 100.0F,
     .health = 100,
     .size = {.x = 64.0F, .y = 64.0F},
     .entityType = rtype::EntityType::EnemyShooter},
}};

/**
 * Get the definition of an enemy by its type.
 * @param type The type of the enemy.
 * @return The definition of the enemy.
 **/
constexpr const EnemyDef &enemyDef(rtype::EnemyType type)
{
    return ENEMY_DEFS.at(static_cast<std::size_t>(type));
}

} // namespace server

#endif // SERVER_ENEMYDEFS_HPP
