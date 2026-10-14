/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnSystem
*/

#ifndef SERVER_SYSTEMS_SPAWNSYSTEM_HPP
#define SERVER_SYSTEMS_SPAWNSYSTEM_HPP

#include "common/components/Collider.hpp"
#include "common/components/Damage.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/NetworkId.hpp"
#include "common/components/Spawner.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "engine/Vec2.hpp"
#include "SpawnRequests.hpp"

#include <random>
#include <vector>

namespace server {

void spawnSystem(engine::Registry &registry, engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_SPAWNSYSTEM_HPP
