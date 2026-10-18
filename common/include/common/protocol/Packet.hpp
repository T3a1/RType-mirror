/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Packet
*/

#ifndef COMMON_PROTOCOL_PACKET_HPP
#define COMMON_PROTOCOL_PACKET_HPP

#include "common/protocol/ByteWriter.hpp"
#include "common/protocol/Messages.hpp"
#include "common/protocol/PacketHeader.hpp"

#include <cstdint>
#include <span>
#include <variant>

namespace rtype::protocol {

/**
 * @brief Every payload a packet can carry, one alternative per PacketType.
 **/
using Payload = std::variant<Connect, Accept, Reject, Disconnect, Ack, Input,
                             Welcome, SnapshotPart, GameEvent>;

/**
 * @struct Packet
 * @brief A decoded packet: its header and its typed payload.
 * @param header The header fields.
 * @param payload The payload, whose alternative matches header.type.
 **/
struct Packet {
    PacketHeader header;
    Payload payload;
    bool operator==(const Packet &) const = default;
};

/**
 * @brief Returns the packet type of the payload held by a variant.
 * @param payload The payload.
 * @return The Type of the alternative it holds.
 **/
[[nodiscard]] PacketType payloadType(const Payload &payload);

/**
 * @brief Appends the header then the payload of a packet. header.type,
 * header.flags and header.payloadSize are computed from the payload, not
 * taken from packet.header.
 * @param packet The packet to write; its sequence and ack are used as is.
 * @param out The writer to append to.
 * @return `true` if the packet fits in MaxPacketSize bytes.
 **/
[[nodiscard]] bool encodePacket(const Packet &packet, ByteWriter &out);

/**
 * @brief Validates and decodes a received datagram: the header, the right
 * of the sender to send its type, then a payload that must use every byte.
 * @param datagram The received bytes.
 * @param sender The side the datagram comes from.
 * @return The packet, or the first validation error found.
 **/
[[nodiscard]] std::variant<Packet, DecodeError>
decodePacket(std::span<const std::uint8_t> datagram, Side sender);

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_PACKET_HPP
