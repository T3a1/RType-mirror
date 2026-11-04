/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnQueue
*/

#ifndef SERVER_COMPONENTS_SPAWNQUEUE_HPP
#define SERVER_COMPONENTS_SPAWNQUEUE_HPP

#include "SpawnRequests.hpp"
#include "engine/Clock.hpp"

#include <vector>

namespace server {

/**
 * @struct PendingSpawn
 * @brief An enemy waiting for its turn in a formation.
 * @param delay The time left before it spawns.
 * @param request What to spawn.
 **/
struct PendingSpawn {
    engine::Duration delay{0.0F};
    SpawnEnemy request;
};

/**
 * @struct SpawnQueue
 * @brief The enemies of the running formations that have not spawned yet.
 * @param pending The enemies, in no particular order.
 **/
struct SpawnQueue {
    std::vector<PendingSpawn> pending;
};

} // namespace server

#endif // SERVER_COMPONENTS_SPAWNQUEUE_HPP
