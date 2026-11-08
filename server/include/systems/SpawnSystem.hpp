/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnSystem
*/

#ifndef SERVER_SYSTEMS_SPAWNSYSTEM_HPP
#define SERVER_SYSTEMS_SPAWNSYSTEM_HPP

#include "EnemyDefs.hpp"
#include "Formations.hpp"
#include "SpawnRequests.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Spawner.hpp"
#include "components/SpawnQueue.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "engine/Vec2.hpp"

#include <cstddef>
#include <random>
#include <vector>

namespace server {

/**
 * @brief Starts random formations at random heights on the right of the
 * playfield, after a random delay, unless levels are being played (the
 * spawner has a LevelProgress), and requests the enemies of the queue when
 * they are due.
 *
 * Reads and writes the countdowns of each entity with a Spawner and a
 * SpawnQueue.
 * @param registry The registry holding the spawners.
 * @param commands Where the enemies that are due are requested.
 * @param dt The time elapsed since the last tick, in seconds.
 **/
void spawnSystem(engine::Registry &registry, SpawnCommands &commands,
                 engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_SPAWNSYSTEM_HPP
