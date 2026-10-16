/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** NetworkInputSystem
*/

#include "systems/NetworkInputSystem.hpp"
#include "common/components/InputState.hpp"

namespace server {

void networkInputSystem(engine::Registry &registry,
                        const std::vector<InboundMessage> &messages,
                        Players &players)
{
    for (const InboundMessage &message : messages) {
        if (message.kind != InboundKind::Input) {
            continue;
        }
        const auto it = players.find(message.clientId);
        if (it == players.end() ||
            message.sequence <= it->second.lastSequence) {
            continue;
        }
        it->second.lastSequence = message.sequence;
        auto *input = registry.tryGet<rtype::InputState>(it->second.ship);
        if (input != nullptr) {
            input->actions = message.actions;
        }
    }
}

} // namespace server
