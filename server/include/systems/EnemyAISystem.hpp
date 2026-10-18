/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EnemyAISystem
*/

#ifndef SERVER_SYSTEMS_ENEMYAISYSTEM_HPP
#define SERVER_SYSTEMS_ENEMYAISYSTEM_HPP

#include "common/Playfield.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Weapon.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

namespace server {
/**
 * @brief Decides what each enemy does this tick.
 *
 * For now, an armed enemy fires while it is inside the playfield: reads
 * Enemy and Transform, writes Weapon::triggered. Runs before the shoot
 * system.
 * @param registry The registry holding the enemies.
 **/
void enemyAISystem(engine::Registry &registry);

} // namespace server

#endif // SERVER_SYSTEMS_ENEMYAISYSTEM_HPP
