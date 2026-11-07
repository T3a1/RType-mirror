/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ChatSystem
*/

#ifndef SERVER_SYSTEMS_CHATSYSTEM_HPP
#define SERVER_SYSTEMS_CHATSYSTEM_HPP

#include "Players.hpp"
#include "network/NetworkMessages.hpp"

#include <vector>

namespace server {

/**
 * @brief Sends the lines of chat of this tick to every player of the room,
 * with the slot of their author, and writes them in the logs. Lines from
 * unknown clients are dropped. It runs in every phase of the room.
 * @param messages The inbound messages of this tick; only Chat is read.
 * @param players The connected players.
 * @param outbound The queue to the network thread.
 **/
void chatSystem(const std::vector<InboundMessage> &messages,
                const Players &players, Outbound &outbound);

} // namespace server

#endif // SERVER_SYSTEMS_CHATSYSTEM_HPP
