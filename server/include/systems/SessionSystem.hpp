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

/**
 * @brief Turns connections into players. On Connected: picks the lowest
 * free slot, spawns the ship with the factory, records the player and
 * sends Welcome to it and PlayerJoined to the others. On Disconnected:
 * despawns the ship, forgets the player and sends PlayerLeft. Runs first in
 * the tick, right after the inbound queue is drained.
 * @param registry The registry the ships live in.
 * @param messages The inbound messages of this tick; Input is ignored.
 * @param players The connected players, updated.
 * @param factory Creates the ships.
 * @param outbound The queue Welcome and the events are pushed to.
 * @param tick The current server tick, sent in Welcome.
 **/
void sessionSystem(engine::Registry &registry,
                   const std::vector<InboundMessage> &messages,
                   Players &players, EntityFactory &factory, Outbound &outbound,
                   std::uint32_t tick);

} // namespace server

#endif // SERVER_SYSTEMS_SESSIONSYSTEM_HPP
