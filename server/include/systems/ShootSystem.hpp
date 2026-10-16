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

void shootSystem(engine::Registry &registry, EntityFactory &factory,
                 engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_SHOOTSYSTEM_HPP
