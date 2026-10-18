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

/**
 * @struct PacketHeader
 * @brief The 12-byte header that starts every packet.
 * @param type The type of the payload.
 * @param flags PacketFlag bits; Reliable must match isReliable(type).
 * @param sequence The sender's reliable or unreliable counter for this
 * packet.
 * @param ack The last reliable sequence received in order from the peer
 * (0 when nothing has been received).
 * @param payloadSize The size of the payload, in bytes.
 **/
struct PacketHeader {
    PacketType type = PacketType::Ack;
    std::uint8_t flags = 0;
    std::uint16_t sequence = 0;
    std::uint16_t ack = 0;
    std::uint16_t payloadSize = 0;

    bool operator==(const PacketHeader &) const = default;
};

/**
 * @enum DecodeError
 * @brief Why a received datagram was rejected, in validation order.
 * @param TooShort Smaller than a header.
 * @param TooLarge Larger than MaxPacketSize.
 * @param BadMagic Does not start with "RT".
 * @param BadVersion Another protocol version.
 * @param UnknownType The type byte is not a PacketType.
 * @param BadFlags Unknown flag bits, or a Reliable flag that does not match
 * the type.
 * @param BadReserved The reserved byte is not 0.
 * @param BadPayloadSize payloadSize does not match the datagram size.
 * @param WrongSender This side may not send this type.
 * @param BadPayload The payload is truncated, too long or out of range.
 **/
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

/**
 * @brief Describes a decode error for the logs.
 * @param error The error to describe.
 * @return A short English description.
 **/
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

/**
 * @brief Writes a header (HeaderSize bytes) with the magic, the protocol
 * version and a zero reserved byte.
 * @param header The fields to write.
 * @param out The writer to append to.
 **/
void encodeHeader(const PacketHeader &header, ByteWriter &out);

/**
 * @brief Reads and validates the header of a datagram.
 * @param datagram The whole received datagram, header and payload.
 * @return The header, or the first validation error found.
 **/
[[nodiscard]] std::variant<PacketHeader, DecodeError>
decodeHeader(std::span<const std::uint8_t> datagram);

/**
 * @brief Compares two 16-bit sequences with serial number arithmetic, so
 * that the comparison stays right when the counter wraps from 65535 to 0.
 * @param a The sequence to test.
 * @param b The reference sequence.
 * @return `true` if `a` is between 1 and 32767 steps after `b`.
 **/
[[nodiscard]] constexpr bool isNewer(std::uint16_t a, std::uint16_t b)
{
    const auto distance = static_cast<std::uint16_t>(a - b);
    return distance != 0 && distance < 0x8000;
}

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_PACKETHEADER_HPP
