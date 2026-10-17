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

void snapshotSystem(engine::Registry &registry, const Players &players,
                    Outbound &outbound, std::uint32_t tick);

} // namespace server

#endif // SERVER_SYSTEMS_SNAPSHOTSYSTEM_HPP
