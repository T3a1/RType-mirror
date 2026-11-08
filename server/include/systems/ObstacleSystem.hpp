/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ObstacleSystem
*/

#ifndef SERVER_SYSTEMS_OBSTACLESYSTEM_HPP
#define SERVER_SYSTEMS_OBSTACLESYSTEM_HPP

#include "SpawnRequests.hpp"
#include "engine/Registry.hpp"

namespace server {

/**
 * @brief Brings in the walls and tiles of the current level: each column of
 * Level::obstacles whose time has come becomes one SpawnObstacle per block,
 * just past the right edge of the playfield. Nothing happens during the
 * pause between two levels or in endless mode. Runs after levelSystem(),
 * which counts the time of the level.
 * @param registry The registry holding the spawner and its LevelProgress.
 * @param commands Where the blocks to create go.
 **/
void obstacleSystem(engine::Registry &registry, SpawnCommands &commands);

} // namespace server

#endif // SERVER_SYSTEMS_OBSTACLESYSTEM_HPP
