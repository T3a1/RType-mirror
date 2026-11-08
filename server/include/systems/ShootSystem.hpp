/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ShootSystem
*/

#ifndef SERVER_SYSTEMS_SHOOTSYSTEM_HPP
#define SERVER_SYSTEMS_SHOOTSYSTEM_HPP

#include "SpawnRequests.hpp"
#include "Weapons.hpp"
#include "common/components/Player.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Weapon.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "engine/Vec2.hpp"

#include <algorithm>
#include <vector>

namespace server {

/**
 * @brief Fires the weapons. An enemy fires one shot to the left whenever
 * its cooldown is over. A player fires when the fire key is pressed (at
 * most once per stats.fireInterval): Weapon::fired is set, and the system
 * of its shot style (ShotStyleSystems.hpp) makes the shots. While the key
 * is held, it only charges; releasing it after stats.chargeTime fires a
 * charged shot that goes through enemies.
 * @param registry The registry holding the weapons.
 * @param commands Where the shots are requested.
 * @param dt The time since the last tick.
 * @param stats The weapon stats, from weapons.json.
 **/
void shootSystem(engine::Registry &registry, SpawnCommands &commands,
                 engine::Duration dt, const WeaponStats &stats);

} // namespace server

#endif // SERVER_SYSTEMS_SHOOTSYSTEM_HPP
