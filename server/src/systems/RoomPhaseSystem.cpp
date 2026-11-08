/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** RoomPhaseSystem
*/

#include "systems/RoomPhaseSystem.hpp"

namespace server {

using rtype::protocol::GameEvent;
using rtype::protocol::GameEventKind;
using rtype::protocol::RoomStatus;

std::optional<rtype::protocol::StartGame>
roomPhaseSystem(const std::vector<InboundMessage> &messages, RoomStatus &status,
                Outbound &outbound)
{
    std::optional<rtype::protocol::StartGame> started;
    for (const InboundMessage &message : messages) {
        if (message.kind == InboundKind::Connected &&
            status == RoomStatus::Playing) {
            outbound.push(OutboundMessage{
                .to = Recipient::One,
                .cliendId = message.clientId,
                .payload = GameEvent{.kind = GameEventKind::GameStarted},
            });
        } else if (message.kind == InboundKind::Start &&
                   status == RoomStatus::Waiting) {
            status = RoomStatus::Playing;
            started = message.settings;
            outbound.push(OutboundMessage{
                .to = Recipient::All,
                .payload = GameEvent{.kind = GameEventKind::GameStarted},
            });
        }
    }
    return started;
}

} // namespace server
