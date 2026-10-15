/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnRequests
*/

#ifndef SERVER_SPAWNREQUESTS_HPP
#define SERVER_SPAWNREQUESTS_HPP

#include "common/components/Enemy.hpp"
#include "engine/Vec2.hpp"

#include <cstdint>

namespace server {

/**
 * A request to spawn an enemy.
 * @param type The type of the enemy to spawn.
 * @param position The position to spawn the enemy at.
 **/
struct SpawnEnemy {
    rtype::EnemyType type = rtype::EnemyType::Basic;
    engine::Vec2f position{};
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

} // namespace server

#endif // SERVER_SPAWNREQUESTS_HPP
