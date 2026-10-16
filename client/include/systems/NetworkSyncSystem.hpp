/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NetworkSyncSystem
*/

#ifndef CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP
#define CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP

#include "common/protocol/Packet.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace client {

struct SyncState {
    std::unordered_map<std::uint32_t, engine::Entity> entities;
    std::unordered_map<std::uint32_t, std::uint32_t> destroyed;
    std::uint32_t lastTick = 0;
    std::uint32_t localShip = 0;
};

void networkSyncSystem(engine::Registry &registry,
                       const std::vector<rtype::protocol::Payload> &payloads,
                       SyncState &state);

} // namespace client

#endif // CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP
