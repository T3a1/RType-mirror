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

void roomPhaseSystem(const std::vector<InboundMessage> &messages,
                     RoomStatus &status, Outbound &outbound)
{
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
            outbound.push(OutboundMessage{
                .to = Recipient::All,
                .payload = GameEvent{.kind = GameEventKind::GameStarted},
            });
        }
    }
}

} // namespace server
