/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PacketType
*/

#ifndef COMMON_PROTOCOL_PACKETTYPE_HPP
#define COMMON_PROTOCOL_PACKETTYPE_HPP

#include <algorithm>
#include <array>
#include <cstdint>

namespace rtype::protocol {

/**
 * @enum PacketType
 * @brief Type byte of a packet header, which also selects its payload.
 * @param Connect A client asks to join (client to server).
 * @param Accept The server accepts a client (reliable).
 * @param Reject The server refuses a client.
 * @param Disconnect Either side ends the connection.
 * @param Ack Acknowledges reliable packets when nothing else is sent.
 * @param Input The actions a player holds (client to server).
 * @param Snapshot One part of the world state at a tick (server to client).
 * @param Welcome The slot and ship of a new player (reliable).
 * @param GameEvent A one-off game fact, such as a destruction (reliable).
 **/
enum class PacketType : std::uint8_t {
    Connect = 0x01,
    Accept = 0x02,
    Reject = 0x03,
    Disconnect = 0x04,
    Ack = 0x05,
    Input = 0x10,
    Snapshot = 0x20,
    Welcome = 0x21,
    GameEvent = 0x22,
};

inline constexpr std::array AllPacketTypes{
    PacketType::Connect,    PacketType::Accept,  PacketType::Reject,
    PacketType::Disconnect, PacketType::Ack,     PacketType::Input,
    PacketType::Snapshot,   PacketType::Welcome, PacketType::GameEvent,
};

/**
 * @brief Checks a type byte read from the network.
 * @param value The raw type byte.
 * @return `true` if it is one of the PacketType values.
 **/
[[nodiscard]] constexpr bool isKnownPacketType(std::uint8_t value)
{
    return std::ranges::any_of(AllPacketTypes, [value](PacketType type) {
        return static_cast<std::uint8_t>(type) == value;
    });
}

/**
 * @enum Side
 * @brief The end of the connection a packet comes from.
 * @param Client A game client.
 * @param Server The game server.
 **/
enum class Side : std::uint8_t {
    Client,
    Server,
};

/**
 * @brief Tells whether a side is allowed to send a packet type.
 * @param side The sender.
 * @param type The type of the packet.
 * @return `true` if the type may come from that side.
 **/
[[nodiscard]] constexpr bool canSend(Side side, PacketType type)
{
    switch (type) {
    case PacketType::Disconnect:
    case PacketType::Ack:
        return true;
    case PacketType::Connect:
    case PacketType::Input:
        return side == Side::Client;
    case PacketType::Accept:
    case PacketType::Reject:
    case PacketType::Snapshot:
    case PacketType::Welcome:
    case PacketType::GameEvent:
        return side == Side::Server;
    }
    return false;
}

/**
 * @brief Tells whether a packet type is sent reliably (numbered,
 * acknowledged and resent until acknowledged).
 * @param type The type of the packet.
 * @return `true` for Accept, Welcome and GameEvent.
 **/
[[nodiscard]] constexpr bool isReliable(PacketType type)
{
    return type == PacketType::Accept || type == PacketType::Welcome ||
           type == PacketType::GameEvent;
}

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_PACKETTYPE_HPP
