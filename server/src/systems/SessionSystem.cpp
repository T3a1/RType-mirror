/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SessionSystem
*/

#include "systems/SessionSystem.hpp"
#include "Despawn.hpp"
#include "common/Playfield.hpp"
#include "common/components/NetworkId.hpp"

#include <algorithm>
#include <optional>

namespace server {

namespace {

/**
 * @brief Finds the lowest slot no connected player uses.
 * @param players The connected players.
 * @return A slot from 1 to MaxPlayers, or nothing if all are taken.
 **/
std::optional<std::uint8_t> freeSlot(const Players &players)
{
    for (std::uint8_t slot = 1; slot <= rtype::protocol::MaxPlayers; slot++) {
        const bool taken =
            std::ranges::any_of(players, [slot](const auto &entry) {
                return entry.second.slot == slot;
            });
        if (!taken) {
            return slot;
        }
    }
    return std::nullopt;
}

/**
 * @brief Returns where the ship of a slot appears: on the left, spread
 * evenly over the playfield height.
 * @param slot The player's slot, from 1 to MaxPlayers.
 * @return The position of the ship's center, in playfield units.
 **/
engine::Vec2f spawnPosition(std::uint8_t slot)
{
    constexpr float SpawnX = 200.0F;
    return {
        .x = SpawnX,
        .y = rtype::PlayfieldHeight * static_cast<float>(slot) /
             static_cast<float>(rtype::protocol::MaxPlayers + 1),
    };
}

/**
 * @brief Spawns the ship of a new client and announces it.
 * @param registry The registry to create the ship in.
 * @param clientId The client that connected; ignored if already a player.
 * @param players The connected players.
 * @param factory Creates the ship.
 * @param outbound The queue Welcome and PlayerJoined are pushed to.
 * @param tick The current server tick, sent in Welcome.
 **/
void onConnected(engine::Registry &registry, std::uint32_t clientId,
                 Players &players, EntityFactory &factory, Outbound &outbound,
                 std::uint32_t tick)
{
    if (players.contains(clientId)) {
        return;
    }
    const auto slot = freeSlot(players);
    if (!slot) {
        return;
    }
    const engine::Entity ship =
        factory.create(registry, SpawnPlayer{
                                     .slot = *slot,
                                     .position = spawnPosition(*slot),
                                 });
    const std::uint32_t shipId = registry.get<rtype::NetworkId>(ship).value;
    players[clientId] = ConnectedPlayer{.ship = ship, .slot = *slot};
    outbound.push(OutboundMessage{
        .to = Recipient::One,
        .cliendId = clientId,
        .payload =
            rtype::protocol::Welcome{
                .slot = *slot,
                .shipId = shipId,
                .tick = tick,
            },
    });
    outbound.push(OutboundMessage{
        .to = Recipient::AllExcept,
        .cliendId = clientId,
        .payload =
            rtype::protocol::GameEvent{
                .kind = rtype::protocol::GameEventKind::PlayerJoined,
                .networkId = shipId,
                .slot = *slot,
            },
    });
}

/**
 * @brief Removes the ship of a client that left and announces it.
 * @param registry The registry holding the ship.
 * @param clientId The client that left; ignored if unknown.
 * @param players The connected players.
 * @param outbound The queue PlayerLeft is pushed to.
 **/
void onDisconnected(engine::Registry &registry, std::uint32_t clientId,
                    Players &players, Outbound &outbound)
{
    const auto it = players.find(clientId);
    if (it == players.end()) {
        return;
    }
    const std::uint8_t slot = it->second.slot;
    despawn(registry, it->second.ship, outbound);
    players.erase(it);
    outbound.push(OutboundMessage{
        .to = Recipient::All,
        .payload =
            rtype::protocol::GameEvent{
                .kind = rtype::protocol::GameEventKind::PlayerLeft,
                .slot = slot,
            },
    });
}

} // namespace

void sessionSystem(engine::Registry &registry,
                   const std::vector<InboundMessage> &messages,
                   Players &players, EntityFactory &factory, Outbound &outbound,
                   std::uint32_t tick)
{
    for (const InboundMessage &message : messages) {
        switch (message.kind) {
        case InboundKind::Connected:
            onConnected(registry, message.clientId, players, factory, outbound,
                        tick);
            break;
        case InboundKind::Disconnected:
            onDisconnected(registry, message.clientId, players, outbound);
            break;
        case InboundKind::Input:
            break;
        }
    }
}

} // namespace server
