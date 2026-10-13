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

using Payload = std::variant<Connect, Accept, Reject, Disconnect, Ack, Input,
                             Welcome, SnapshotPart, GameEvent>;

struct Packet {
    PacketHeader header;
    Payload payload;
    bool operator==(const Packet &) const = default;
};

// Appends the header then the payload to out. header.type, header.flags
// and header.payloadSize are taken from the payload, not from
// packet.header. False if the packet does not fit in MaxPacketSize.
[[nodiscard]] bool encodePacket(const Packet &packet, ByteWriter &out);

// The only entry point for received bytes: checks the header and that
// sender may send this type, then decodes the payload, which must use
// every byte.
[[nodiscard]] std::variant<Packet, DecodeError>
decodePacket(std::span<const std::uint8_t> datagram, Side sender);

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_PACKET_HPP
