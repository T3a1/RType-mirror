/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NetworkSyncSystem
*/

#ifndef CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP
#define CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP

#include "common/protocol/Packet.hpp"
#include "common/systems/InterpolationSystem.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

#include <cstdint>
#include <unordered_map>
#include <utility>
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
 * @param clock The server tick the synced entities are drawn at.
 * @param dying Destroyed entities still drawn until the render clock
 * reaches the given tick: the tick they were destroyed at, minus their
 * lead.
 **/
struct SyncState {
    std::unordered_map<std::uint32_t, engine::Entity> entities;
    std::unordered_map<std::uint32_t, std::uint32_t> destroyed;
    std::uint32_t lastTick = 0;
    std::uint32_t localShip = 0;
    std::uint8_t localSlot = 0;
    std::vector<rtype::protocol::PlayerInfo> players;
    rtype::RenderClock clock;
    std::vector<std::pair<engine::Entity, double>> dying;
};

/**
 * @brief Applies what the server sent to the local registry. A snapshot
 * creates unknown entities (NetworkId, EntityType, Transform, Health,
 * Sprite, and a SnapshotHistory except for the local ship) and adds to the
 * history of known ones, or updates the Transform and Health of the local
 * ship;
 * snapshots older than the last one are ignored. EntityDestroyed hands the
 * entity to the dying list; Welcome marks the local ship. Runs first in the
 * frame, on the payloads of NetworkClient::update().
 * @param registry The client's registry.
 * @param payloads The payloads received this frame, in order.
 * @param state The sync state, updated.
 **/
void networkSyncSystem(engine::Registry &registry,
                       const std::vector<rtype::protocol::Payload> &payloads,
                       SyncState &state);

/**
 * @brief Advances the render clock, moves the synced entities to their
 * interpolated position at its tick, and destroys the dying entities whose
 * tick it reached. Runs right after networkSyncSystem().
 * @param registry The client's registry.
 * @param state The sync state.
 * @param dt The time elapsed since the last frame.
 **/
void smoothingSystem(engine::Registry &registry, SyncState &state,
                     engine::Duration dt);

} // namespace client

#endif // CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP
