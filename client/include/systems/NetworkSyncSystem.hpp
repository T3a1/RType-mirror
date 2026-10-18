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

/**
 * @struct SyncState
 * @brief What the client knows about the server's world.
 * @param entities The local entity of each NetworkId.
 * @param destroyed Recently destroyed NetworkIds and the tick they were
 * destroyed at, so that a late snapshot cannot bring them back.
 * @param lastTick The newest snapshot tick applied.
 * @param localShip The NetworkId of the local player's ship, 0 until
 * Welcome.
 * @param localSlot The local player's slot, 0 until Welcome.
 * @param players The players block of the last snapshot.
 **/
struct SyncState {
    std::unordered_map<std::uint32_t, engine::Entity> entities;
    std::unordered_map<std::uint32_t, std::uint32_t> destroyed;
    std::uint32_t lastTick = 0;
    std::uint32_t localShip = 0;
    std::uint8_t localSlot = 0;
    std::vector<rtype::protocol::PlayerInfo> players;
};

/**
 * @brief Applies what the server sent to the local registry. A snapshot
 * creates unknown entities (NetworkId, EntityType, Transform, Health,
 * Sprite) and updates the Transform and Health of known ones; snapshots
 * older than the last one are ignored. EntityDestroyed destroys the entity;
 * Welcome marks the local ship. Runs first in the frame, on the payloads of
 * NetworkClient::update().
 * @param registry The client's registry.
 * @param payloads The payloads received this frame, in order.
 * @param state The sync state, updated.
 **/
void networkSyncSystem(engine::Registry &registry,
                       const std::vector<rtype::protocol::Payload> &payloads,
                       SyncState &state);

} // namespace client

#endif // CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP
