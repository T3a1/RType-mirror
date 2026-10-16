/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** NetworkInputSystem
*/

#ifndef SERVER_SYSTEMS_NETWORKINPUTSYSTEM_HPP
#define SERVER_SYSTEMS_NETWORKINPUTSYSTEM_HPP
#include "Players.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"

#include <vector>

namespace server {

void networkInputSystem(engine::Registry &registry,
                        const std::vector<InboundMessage> &messages,
                        Players &players);

} // namespace server

#endif // SERVER_SYSTEMS_NETWORKINPUTSYSTEM_HPP
