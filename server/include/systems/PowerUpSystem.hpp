/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** PowerUpSystem
*/

#ifndef SERVER_SYSTEMS_POWERUPSYSTEM_HPP
#define SERVER_SYSTEMS_POWERUPSYSTEM_HPP

#include "EntityFactory.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"
#include "systems/CollisionSystem.hpp"

#include <cstdint>
#include <vector>

namespace server {

/**
 * @brief Drops a power-up where each enemy that drops one (EnemyDef::
 * dropsPowerUp) was destroyed this tick. Runs after damageSystem() and
 * before deathSystem(), while the dead enemies are still there.
 * @param registry The registry holding the enemies.
 * @param factory The factory that creates the power-ups.
 **/
void powerUpDropSystem(engine::Registry &registry, EntityFactory &factory);

/**
 * @brief Gives one more weapon level, up to maxLevel, to each player who
 * touches a power-up this tick, and removes the power-up.
 * @param registry The registry holding the players and the power-ups.
 * @param collisions The collisions of this tick.
 * @param outbound Where the removal of the power-ups is announced.
 * @param maxLevel The highest weapon level.
 **/
void pickupSystem(engine::Registry &registry,
                  const std::vector<Collision> &collisions, Outbound &outbound,
                  std::uint8_t maxLevel);

} // namespace server

#endif // SERVER_SYSTEMS_POWERUPSYSTEM_HPP
