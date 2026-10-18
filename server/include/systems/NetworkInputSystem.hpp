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

/**
 * @brief Writes the newest input of each player into the InputState of its
 * ship. Inputs from unknown clients, or not newer than the last one applied,
 * are dropped. Runs right after sessionSystem, before the systems that read
 * InputState.
 * @param registry The registry the ships live in.
 * @param messages The inbound messages of this tick; only Input is read.
 * @param players The connected players; lastSequence is updated.
 **/
void networkInputSystem(engine::Registry &registry,
                        const std::vector<InboundMessage> &messages,
                        Players &players);

} // namespace server

#endif // SERVER_SYSTEMS_NETWORKINPUTSYSTEM_HPP
