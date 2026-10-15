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

enum class PacketType : std::uint8_t {
    Connect = 0x01,
    Accept = 0x02,
    Reject = 0x03,
    Disconnect = 0x04,
    Ack = 0x05,
    Input = 0x10,
    Snapshot = 0x20,
    Welcome = 0x21,
    GameEvent = 0x22
};

inline constexpr std::array AllPacketTypes{
    PacketType::Connect,    PacketType::Accept,  PacketType::Reject,
    PacketType::Disconnect, PacketType::Ack,     PacketType::Input,
    PacketType::Snapshot,   PacketType::Welcome, PacketType::GameEvent,
};

[[nodiscard]] constexpr bool isKnownPacketType(std::uint8_t value)
{
    return std::ranges::any_of(AllPacketTypes, [value](PacketType type) {
        return static_cast<std::uint8_t>(type) == value;
    });
}

enum class Side : std::uint8_t {
    Client,
    Server,
};

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

[[nodiscard]] constexpr bool isReliable(PacketType type)
{
    return type == PacketType::Accept || type == PacketType::Welcome ||
           type == PacketType::GameEvent;
}

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_PACKETTYPE_HPP
