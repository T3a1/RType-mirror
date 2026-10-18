/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SnapshotSystem
*/

#ifndef SERVER_SYSTEMS_SNAPSHOTSYSTEM_HPP
#define SERVER_SYSTEMS_SNAPSHOTSYSTEM_HPP
#include "Players.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"

#include <cstdint>

namespace server {

/**
 * @brief Sends the state of the world to every client: the players and
 * every entity with a NetworkId, an EntityType and a Transform, split into
 * parts that fit in a datagram. Called every third tick (20 times per
 * second), after the registry is flushed.
 * @param registry The registry to read.
 * @param players The connected players, for the players block.
 * @param outbound The queue the parts are pushed to.
 * @param tick The current server tick.
 **/
void snapshotSystem(engine::Registry &registry, const Players &players,
                    Outbound &outbound, std::uint32_t tick);

} // namespace server

#endif // SERVER_SYSTEMS_SNAPSHOTSYSTEM_HPP
