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

/**
 * @brief Removes every entity whose health has run out.
 * @param registry The registry holding the entities.
 * @param outbound Where the deaths are announced to the clients.
 **/
void deathSystem(engine::Registry &registry, Outbound &outbound);

} // namespace server

#endif // SERVER_SYSTEMS_DEATHSYSTEM_HPP
