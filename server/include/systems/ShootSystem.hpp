/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ShootSystem
*/

#ifndef SERVER_SYSTEMS_SHOOTSYSTEM_HPP
#define SERVER_SYSTEMS_SHOOTSYSTEM_HPP

#include "EntityFactory.hpp"
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
 * @brief Fires the weapons whose cooldown is over. An enemy fires one shot
 * to the left. A player fires 2 * level - 1 shots in a spread; holding the
 * key also charges, and releasing it after stats.chargeTime fires a
 * charged shot that goes through enemies.
 * @param registry The registry holding the weapons.
 * @param factory The factory that creates the shots.
 * @param dt The time since the last tick.
 * @param stats The weapon stats, from weapons.json.
 **/
void shootSystem(engine::Registry &registry, EntityFactory &factory,
                 engine::Duration dt, const WeaponStats &stats);

} // namespace server

#endif // SERVER_SYSTEMS_SHOOTSYSTEM_HPP
