/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** PowerUpSystem
*/

#ifndef SERVER_SYSTEMS_POWERUPSYSTEM_HPP
#define SERVER_SYSTEMS_POWERUPSYSTEM_HPP

#include "SpawnRequests.hpp"
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
 * @param commands Where the power-ups are requested.
 **/
void powerUpDropSystem(engine::Registry &registry, SpawnCommands &commands);

/**
 * @brief Gives the shot style of a power-up, or one more level of it up
 * to maxLevel, to each player who touches one this tick, and removes the
 * power-up.
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
