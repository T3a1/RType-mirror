/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** DebugSpawnSystem
*/

#ifndef CLIENT_SYSTEMS_DEBUGSPAWNSYSTEM_HPP
#define CLIENT_SYSTEMS_DEBUGSPAWNSYSTEM_HPP

#include "engine/Registry.hpp"

#include <random>

namespace client {

/**
 * @brief Spawns a random enemy each time SPACE is pressed.
 *
 * Picks a random enemy type and a random height that keeps the whole enemy
 * inside the window, then calls spawnEnemy(). Temporary: the server will
 * send the enemies to spawn.
 *
 * @param registry Registry to create the enemy in.
 * @param random Random generator used for the type and the height.
 */
void debugSpawnSystem(engine::Registry &registry, std::mt19937 &random);

} // namespace client

#endif // CLIENT_SYSTEMS_DEBUGSPAWNSYSTEM_HPP
