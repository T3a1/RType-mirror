/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ChatSystem
*/

#include "systems/ChatSystem.hpp"
#include "engine/Logger.hpp"

namespace server {

void chatSystem(const std::vector<InboundMessage> &messages,
                const Players &players, Outbound &outbound)
{
    for (const InboundMessage &message : messages) {
        if (message.kind != InboundKind::Chat) {
            continue;
        }
        const auto it = players.find(message.clientId);
        if (it == players.end()) {
            continue;
        }
        BB_INFO("Chat from client " << message.clientId << ": "
                                    << message.text);
        outbound.push(OutboundMessage{
            .to = Recipient::All,
            .payload =
                rtype::protocol::ChatMessage{
                    .slot = it->second.slot,
                    .text = message.text,
                },
        });
    }
}

} // namespace server
