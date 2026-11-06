/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnRequests
*/

#ifndef SERVER_SPAWNREQUESTS_HPP
#define SERVER_SPAWNREQUESTS_HPP

#include "common/components/Enemy.hpp"
#include "common/components/MovementPattern.hpp"
#include "components/ShotStyles.hpp"
#include "engine/Entity.hpp"
#include "engine/Vec2.hpp"

#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

namespace server {

/**
 * @struct SpawnEnemy
 * @brief A request to spawn an enemy.
 * @param type The type of the enemy to spawn.
 * @param position The position to spawn the enemy at.
 * @param pattern The path it follows, or nothing for the one of its type.
 **/
struct SpawnEnemy {
    rtype::EnemyType type = rtype::EnemyType::Basic;
    engine::Vec2f position{};
    std::optional<rtype::MovementPattern> pattern;
};

/**
 * @struct SpawnPlayer
 * @brief A request to spawn a player.
 * @param slot The slot of the player to spawn.
 * @param position The position to spawn the player at.
 **/
struct SpawnPlayer {
    std::uint8_t slot = 1;
    engine::Vec2f position{};
};

/**
 * @struct SpawnProjectile
 * @brief A request to spawn a projectile.
 * @param owner The entity that owns the projectile.
 * @param position The position to spawn the projectile at.
 * @param velocity The velocity of the projectile.
 * @param fromPlayer Whether the projectile was fired by a player or an enemy.
 * @param damage The damage it deals.
 * @param charged Whether it is a player's charged shot: larger, and it goes
 * through the enemies it hits.
 **/
struct SpawnProjectile {
    engine::Entity owner{};
    engine::Vec2f position{};
    engine::Vec2f velocity{};
    bool fromPlayer = true;
    int damage = 50;
    bool charged = false;
};

/**
 * @struct SpawnPowerUp
 * @brief A request to spawn a power-up, dropped by a destroyed enemy.
 * @param position Where it appears.
 * @param kind The shot style it gives.
 **/
struct SpawnPowerUp {
    engine::Vec2f position{};
    PowerUpKind kind = PowerUpKind::Spread;
};

/**
 * @struct SpawnObstacle
 * @brief A request to spawn one block of the level.
 * @param position The center of the block.
 * @param destroyable Whether it is a destroyable tile instead of a wall.
 **/
struct SpawnObstacle {
    engine::Vec2f position{};
    bool destroyable = false;
};

/** @brief A deferred entity creation. */
using SpawnCommand =
    std::variant<SpawnEnemy, SpawnProjectile, SpawnPowerUp, SpawnObstacle>;

/** @brief The creations requested during a tick, applied in the Cleanup stage.
 */
using SpawnCommands = std::vector<SpawnCommand>;

/** @brief Distance between the center of a ship and the start of its shots. */
inline constexpr float MuzzleOffset = 40.0F;

} // namespace server

#endif // SERVER_SPAWNREQUESTS_HPP
