/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** PowerUpSystem
*/

#ifndef SERVER_SYSTEMS_POWERUPSYSTEM_HPP
#define SERVER_SYSTEMS_POWERUPSYSTEM_HPP

#include "SpawnRequests.hpp"
#include "Weapons.hpp"
#include "engine/Clock.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"
#include "systems/CollisionSystem.hpp"

#include <random>
#include <vector>

namespace server {

/**
 * @brief Drops a power-up of a random kind where each enemy that drops one
 * (EnemyDef::dropsPowerUp) was destroyed this tick. Runs after
 * damageSystem() and before deathSystem(), while the dead enemies are
 * still there.
 * @param registry The registry holding the enemies.
 * @param commands Where the power-ups are requested.
 * @param random The generator that picks the kinds.
 **/
void powerUpDropSystem(engine::Registry &registry, SpawnCommands &commands,
                       std::mt19937 &random);

/**
 * @brief Gives the boost of a power-up to each player who touches one this
 * tick, and removes the power-up. The boost lasts its full duration, even
 * if the player already had it; boosts of different kinds stack. A Spread
 * boost also raises the spread level, up to stats.maxLevel.
 * @param registry The registry holding the players and the power-ups.
 * @param collisions The collisions of this tick.
 * @param outbound Where the removal of the power-ups is announced.
 * @param stats The weapon stats.
 **/
void pickupSystem(engine::Registry &registry,
                  const std::vector<Collision> &collisions, Outbound &outbound,
                  const WeaponStats &stats);

/**
 * @brief Counts down the boosts of the players and takes away the ones
 * whose time is up: the spread goes back to a single shot, the aimed and
 * burst styles are removed.
 * @param registry The registry holding the players.
 * @param dt The time since the last tick.
 **/
void boostSystem(engine::Registry &registry, engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_POWERUPSYSTEM_HPP
