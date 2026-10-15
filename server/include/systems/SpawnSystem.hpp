/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnSystem
*/

#ifndef SERVER_SYSTEMS_SPAWNSYSTEM_HPP
#define SERVER_SYSTEMS_SPAWNSYSTEM_HPP

#include "EntityFactory.hpp"
#include "SpawnRequests.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Spawner.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "engine/Vec2.hpp"

#include <random>
#include <vector>

namespace server {

void spawnSystem(engine::Registry &registry, EntityFactory &factory, engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_SPAWNSYSTEM_HPP
