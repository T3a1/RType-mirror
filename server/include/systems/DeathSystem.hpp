/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** DeathSystem
*/

#ifndef SERVER_SYSTEMS_DEATHSYSTEM_HPP
#define SERVER_SYSTEMS_DEATHSYSTEM_HPP

#include "Despawn.hpp"
#include "common/components/Health.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"

namespace server {

void deathSystem(engine::Registry &registry, Outbound &outbound);

} // namespace server

#endif // SERVER_SYSTEMS_DEATHSYSTEM_HPP
