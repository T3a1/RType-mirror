/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** NetworkMessages
*/

#ifndef SERVER_NETWORKMESSAGES_HPP
#define SERVER_NETWORKMESSAGES_HPP
#include "common/protocol/Packet.hpp"
#include "engine/MessageQueue.hpp"
#include <cstdint>

namespace server {

enum class Recipient : std::uint8_t { All, AllExcept, One };
enum class InboundKind : std::uint8_t { Connected, Disconnected, Input };

struct InboundMessage {
    std::uint32_t clientId = 0;
    InboundKind kind = InboundKind::Input;
    rtype::protocol::DisconnectReason reason =
        rtype::protocol::DisconnectReason::Timeout;
    std::uint32_t sequence = 0;
    std::uint8_t actions = 0;
};

struct OutboundMessage {
    Recipient to = Recipient::All;
    std::uint32_t cliendId = 0;
    rtype::protocol::Payload payload = rtype::protocol::Accept{};
};

using Inbound = engine::MessageQueue<InboundMessage>;
using Outbound = engine::MessageQueue<OutboundMessage>;

} // namespace server

#endif // SERVER_NETWORKMESSAGES_HPP
