/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Despawn
*/

#include "Despawn.hpp"
#include "common/components/NetworkId.hpp"

namespace server {

void despawn(engine::Registry &registry, engine::Entity entity,
             Outbound &outbound)
{
    if (const auto *networkId = registry.tryGet<rtype::NetworkId>(entity)) {
        outbound.push(OutboundMessage{
            .to = Recipient::All,
            .payload =
                rtype::protocol::GameEvent{
                    .kind = rtype::protocol::GameEventKind::EntityDestroyed,
                    .networkId = networkId->value,
                },
        });
    }
    registry.destroy(entity);
}

} // namespace server
