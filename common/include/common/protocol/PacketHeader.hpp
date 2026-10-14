/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PacketHeader
*/

#ifndef COMMON_PROTOCOL_PACKETHEADER_HPP
#define COMMON_PROTOCOL_PACKETHEADER_HPP

#include "common/protocol/ByteWriter.hpp"
#include "common/protocol/PacketType.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <variant>

namespace rtype::protocol {

inline constexpr std::array<std::uint8_t, 2> Magic{'R', 'T'};
inline constexpr std::uint8_t ProtocolVersion = 1;
inline constexpr std::size_t HeaderSize = 12;
inline constexpr std::size_t MaxPayloadSize = MaxPacketSize - HeaderSize;
inline constexpr std::size_t PayloadSizeOffset = 10;

namespace PacketFlag {
inline constexpr std::uint8_t Reliable = 1U << 0U;
inline constexpr std::uint8_t All = Reliable;
} // namespace PacketFlag

struct PacketHeader {
    PacketType type = PacketType::Ack;
    std::uint8_t flags = 0;
    std::uint16_t sequence = 0;
    std::uint16_t ack = 0;
    std::uint16_t payloadSize = 0;

    bool operator==(const PacketHeader &) const = default;
};

enum class DecodeError : std::uint8_t {
    TooShort,
    TooLarge,
    BadMagic,
    BadVersion,
    UnknownType,
    BadFlags,
    BadReserved,
    BadPayloadSize,
    WrongSender,
    BadPayload,
};

constexpr std::string_view toString(DecodeError error)
{
    switch (error) {
    case DecodeError::TooShort:
        return "Packet is too short";
    case DecodeError::TooLarge:
        return "Packet is too large";
    case DecodeError::BadMagic:
        return "Bad magic number";
    case DecodeError::BadVersion:
        return "Bad protocol version";
    case DecodeError::UnknownType:
        return "Unknown packet type";
    case DecodeError::BadFlags:
        return "Bad packet flags";
    case DecodeError::BadReserved:
        return "Bad reserved bytes";
    case DecodeError::BadPayloadSize:
        return "Bad payload size";
    case DecodeError::WrongSender:
        return "Wrong sender";
    case DecodeError::BadPayload:
        return "Bad payload";
    }
    return {};
}

void encodeHeader(const PacketHeader &header, ByteWriter &out);
[[nodiscard]] std::variant<PacketHeader, DecodeError>
decodeHeader(std::span<const std::uint8_t> datagram);

[[nodiscard]] constexpr bool isNewer(std::uint16_t a, std::uint16_t b)
{
    const auto distance = static_cast<std::uint16_t>(a - b);
    return distance != 0 && distance < 0x8000;
}

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_PACKETHEADER_HPP
