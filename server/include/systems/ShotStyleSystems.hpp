/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ShotStyleSystems
*/

#ifndef SERVER_SYSTEMS_SHOTSTYLESYSTEMS_HPP
#define SERVER_SYSTEMS_SHOTSTYLESYSTEMS_HPP

#include "SpawnRequests.hpp"
#include "Weapons.hpp"
#include "engine/Registry.hpp"

namespace server {

// One system per shot style (components/ShotStyles.hpp). Each runs after
// shootSystem() and turns the Weapon::fired of the players that have its
// style into projectiles. burstShotSystem() runs first: it sets
// Weapon::fired again for the next shots of a burst.

/**
 * @brief Fires the bursts of the players with a BurstShot: after each
 * shot, sets Weapon::fired again stats.burstShots times, stats.burstDelay
 * apart, so that every other style fires with it.
 * @param registry The registry holding the players.
 * @param dt The time since the last tick.
 * @param stats The weapon stats.
 **/
void burstShotSystem(engine::Registry &registry, engine::Duration dt,
                     const WeaponStats &stats);

/**
 * @brief Fires the shots of the players with a SpreadShot: 2 * level - 1
 * shots, stats.spreadAngle apart.
 * @param registry The registry holding the players.
 * @param commands Where the shots are requested.
 * @param stats The weapon stats.
 **/
void spreadShotSystem(engine::Registry &registry, SpawnCommands &commands,
                      const WeaponStats &stats);

/**
 * @brief Fires the shots of the players with an AimedShot: one shot towards
 * the nearest enemy, straight ahead if there is none.
 * @param registry The registry holding the players and the enemies.
 * @param commands Where the shots are requested.
 * @param stats The weapon stats.
 **/
void aimedShotSystem(engine::Registry &registry, SpawnCommands &commands,
                     const WeaponStats &stats);

} // namespace server

#endif // SERVER_SYSTEMS_SHOTSTYLESYSTEMS_HPP
