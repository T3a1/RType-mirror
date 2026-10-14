/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** NetworkMessages
*/

#ifndef SERVER_NETWORKMESSAGES_HPP
#define SERVER_NETWORKMESSAGES_HPP
#include "common/protocol/Messages.hpp"
#include "common/protocol/Packet.hpp"
#include <cstdint>

// C
// typedef enum { All, AllExcept, One } receivers;

namespace server {

enum class Recipients : std::uint8_t { All, AllExcept, One };
enum class InboundKind { Connected, Disconnected, In };

struct InboundMessage {
    std::uint32_t clientId;
    Recipients recipients;
    rtype::protocol::DisconnectReason reason;
    std::uint8_t actions;
};

struct OutboundMessage {
    Recipients to;
    std::uint32_t cliendId;
    rtype::protocol::Payload payload;
};

} // namespace server

#endif
