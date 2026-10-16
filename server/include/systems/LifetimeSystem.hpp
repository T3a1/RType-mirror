/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** LifetimeSystem
*/

#ifndef SERVER_SYSTEMS_LIFETIMESYSTEM_HPP
#define SERVER_SYSTEMS_LIFETIMESYSTEM_HPP

#include "common/components/Lifetime.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"

namespace server {

void lifetimeSystem(engine::Registry &registry, engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_LIFETIMESYSTEM_HPP
