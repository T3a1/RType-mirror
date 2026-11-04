/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** RoomPhaseSystem
*/

#ifndef SERVER_SYSTEMS_ROOMPHASESYSTEM_HPP
#define SERVER_SYSTEMS_ROOMPHASESYSTEM_HPP

#include "common/protocol/Messages.hpp"
#include "network/NetworkMessages.hpp"

#include <vector>

namespace server {

/**
 * @brief Starts the game of a waiting room when a player asks for it, which
 * every player learns from GameEvent{GameStarted}. A player who enters a room
 * whose game already runs receives it alone.
 * @param messages The inbound messages of this tick.
 * @param status Where the game stands, updated.
 * @param outbound The queue the events are pushed to.
 **/
void roomPhaseSystem(const std::vector<InboundMessage> &messages,
                     rtype::protocol::RoomStatus &status, Outbound &outbound);

} // namespace server

#endif // SERVER_SYSTEMS_ROOMPHASESYSTEM_HPP
