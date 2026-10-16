/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SessionSystem
*/

#ifndef SERVER_SYSTEMS_SESSIONSYSTEM_HPP
#define SERVER_SYSTEMS_SESSIONSYSTEM_HPP
#include "EntityFactory.hpp"
#include "Players.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"

namespace server {

void sessionSystem(engine::Registry const &registry,
                   const std::vector<InboundMessage> &messages,
                   Players const &players, EntityFactory const &factory,
                   Outbound const &outbound, std::uint32_t tick);

} // namespace server

#endif // SERVER_SYSTEMS_SESSIONSYSTEM_HPP
