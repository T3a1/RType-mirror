/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** DamageSystem
*/

#ifndef SERVER_SYSTEMS_DAMAGESYSTEM_HPP
#define SERVER_SYSTEMS_DAMAGESYSTEM_HPP

#include "Despawn.hpp"
#include "EnemyDefs.hpp"
#include "common/components/Damage.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Health.hpp"
#include "common/components/Player.hpp"
#include "common/components/Projectile.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"
#include "systems/CollisionSystem.hpp"

#include <algorithm>
#include <vector>

namespace server {

/**
 * @brief Applies the effects of every collision of this tick, in both
 * directions.
 * @param registry The registry holding the entities.
 * @param collisions The pairs found by the collision system.
 * @param outbound Where the destructions are announced to the clients.
 * @param invinciblePlayers Whether the players' ships take no damage, for
 * the network benchmark.
 **/
void damageSystem(engine::Registry &registry,
                  const std::vector<Collision> &collisions, Outbound &outbound,
                  bool invinciblePlayers = false);

} // namespace server

#endif // SERVER_SYSTEMS_DAMAGESYSTEM_HPP
