/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SnapshotSystem
*/

#include "systems/SnapshotSystem.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/NetworkId.hpp"
#include "common/components/Player.hpp"
#include "common/components/Transform.hpp"
#include "common/protocol/Messages.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace server {

namespace {

std::uint16_t healthOf(engine::Registry &registry, engine::Entity entity)
{
    const auto *health = registry.tryGet<rtype::Health>(entity);
    if (health == nullptr) {
        return 0;
    }
    return static_cast<std::uint16_t>(std::clamp(
        health->current, 0, int{std::numeric_limits<std::uint16_t>::max()}));
}

std::vector<rtype::protocol::EntityState>
entityStates(engine::Registry &registry)
{
    std::vector<rtype::protocol::EntityState> states;
    registry.each<rtype::NetworkId, rtype::EntityType, rtype::Transform>(
        [&](engine::Entity entity, const rtype::NetworkId &networkId,
            const rtype::EntityType &type, const rtype::Transform &transform) {
            states.push_back({
                .networkId = networkId.value,
                .type = type,
                .health = healthOf(registry, entity),
                .x = transform.position.x,
                .y = transform.position.y,
            });
        });
    return states;
}

std::vector<rtype::protocol::PlayerInfo> playerInfos(engine::Registry &registry,
                                                     const Players &players)
{
    std::vector<rtype::protocol::PlayerInfo> infos;
    for (const auto &[clientId, player] : players) {
        const auto *component = registry.tryGet<rtype::Player>(player.ship);
        infos.push_back({
            .slot = player.slot,
            .score = component != nullptr ? component->score : 0,
        });
    }
    return infos;
}

} // namespace

void snapshotSystem(engine::Registry &registry, const Players &players,
                    Outbound &outbound, std::uint32_t tick)
{
    const auto entities = entityStates(registry);
    const auto infos = playerInfos(registry, players);
    for (rtype::protocol::SnapshotPart &part :
         rtype::protocol::splitSnapshot(tick, infos, entities)) {
        outbound.push(OutboundMessage{
            .to = Recipient::All,
            .payload = std::move(part),
        });
    }
}

} // namespace server
