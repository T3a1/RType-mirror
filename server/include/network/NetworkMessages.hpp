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

/**
 * @enum Recipient
 * @brief Which clients an outbound message is sent to.
 * @param All Every connected client.
 * @param AllExcept Every connected client but OutboundMessage::cliendId.
 * @param One Only OutboundMessage::cliendId.
 **/
enum class Recipient : std::uint8_t { All, AllExcept, One };

/**
 * @enum InboundKind
 * @brief What an inbound message tells the game thread.
 * @param Connected A client finished the handshake.
 * @param Disconnected A client left, timed out or was kicked.
 * @param Input A newer input from a client.
 **/
enum class InboundKind : std::uint8_t { Connected, Disconnected, Input };

/**
 * @struct InboundMessage
 * @brief A fact pushed by the network thread to the game thread. It never
 * contains an address: clients are named by their ClientId.
 * @param clientId The client concerned.
 * @param kind What happened.
 * @param reason Why the client left, for Disconnected.
 * @param sequence The input sequence number, for Input.
 * @param actions The held actions as Action bits, for Input.
 **/
struct InboundMessage {
    std::uint32_t clientId = 0;
    InboundKind kind = InboundKind::Input;
    rtype::protocol::DisconnectReason reason =
        rtype::protocol::DisconnectReason::Timeout;
    std::uint32_t sequence = 0;
    std::uint8_t actions = 0;
};

/**
 * @struct OutboundMessage
 * @brief A payload pushed by the game thread for the network thread to
 * send. The network thread picks the delivery mode from the payload type.
 * @param to Which clients receive it.
 * @param cliendId The client used by One and AllExcept.
 * @param payload What to send.
 **/
struct OutboundMessage {
    Recipient to = Recipient::All;
    std::uint32_t cliendId = 0;
    rtype::protocol::Payload payload = rtype::protocol::Accept{};
};

/**
 * @brief Queue from the network thread to the game thread.
 **/
using Inbound = engine::MessageQueue<InboundMessage>;

/**
 * @brief Queue from the game thread to the network thread.
 **/
using Outbound = engine::MessageQueue<OutboundMessage>;

} // namespace server

#endif // SERVER_NETWORKMESSAGES_HPP
