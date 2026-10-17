/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** DamageSystem
*/

#ifndef SERVER_SYSTEMS_DAMAGESYSTEM_HPP
#define SERVER_SYSTEMS_DAMAGESYSTEM_HPP

#include "Despawn.hpp"
#include "common/components/Damage.hpp"
#include "common/components/Health.hpp"
#include "common/components/Projectile.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"
#include "systems/CollisionSystem.hpp"

#include <algorithm>
#include <vector>

namespace server {

void damageSystem(engine::Registry &registry,
                  const std::vector<Collision> &collisions, Outbound &outbound);

} // namespace server

#endif // SERVER_SYSTEMS_DAMAGESYSTEM_HPP
