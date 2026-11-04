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
#include "engine/Entity.hpp"
#include "engine/Vec2.hpp"

#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

namespace server {

/**
 * A request to spawn an enemy.
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
 * A request to spawn a player.
 * @param slot The slot of the player to spawn.
 * @param position The position to spawn the player at.
 **/
struct SpawnPlayer {
    std::uint8_t slot = 1;
    engine::Vec2f position{};
};

/**
 * A request to spawn a projectile.
 * @param owner The entity that owns the projectile.
 * @param position The position to spawn the projectile at.
 * @param velocity The velocity of the projectile.
 * @param fromPlayer Whether the projectile was fired by a player or an enemy.
 **/
struct SpawnProjectile {
    engine::Entity owner{};
    engine::Vec2f position{};
    engine::Vec2f velocity{};
    bool fromPlayer = true;
};

/// A deferred entity creation.
using SpawnCommand = std::variant<SpawnEnemy, SpawnProjectile>;

/// The creations requested during a tick, applied in the Cleanup stage.
using SpawnCommands = std::vector<SpawnCommand>;

} // namespace server

#endif // SERVER_SPAWNREQUESTS_HPP
