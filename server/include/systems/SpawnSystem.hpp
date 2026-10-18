/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnSystem
*/

#ifndef SERVER_SYSTEMS_SPAWNSYSTEM_HPP
#define SERVER_SYSTEMS_SPAWNSYSTEM_HPP

#include "EnemyDefs.hpp"
#include "EntityFactory.hpp"
#include "SpawnRequests.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Spawner.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "engine/Vec2.hpp"

#include <cstddef>
#include <random>
#include <vector>

namespace server {

/**
 * @brief Spawns enemies of random types at random heights on the right of
 * the playfield, after a random delay.
 *
 * Reads and writes each Spawner's countdown, then creates the enemies with
 * the factory after the loop.
 * @param registry The registry holding the spawners.
 * @param factory The factory that creates the enemies.
 * @param dt The time elapsed since the last tick, in seconds.
 **/
void spawnSystem(engine::Registry &registry, EntityFactory &factory,
                 engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_SPAWNSYSTEM_HPP
