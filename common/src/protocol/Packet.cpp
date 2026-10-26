/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Packet
*/

#include "common/protocol/Packet.hpp"
#include "common/protocol/ByteReader.hpp"

#include <type_traits>

namespace rtype::protocol {

namespace {

/**
 * @brief Decodes a payload of a given type and builds the packet.
 * @param header The validated header of the packet.
 * @param in The reader positioned at the start of the payload.
 * @return The packet, or BadPayload if the payload is invalid, truncated or
 * followed by extra bytes.
 **/
template <typename Message>
std::variant<Packet, DecodeError> decodePayload(const PacketHeader &header,
                                                ByteReader &in)
{
    Message message;
    if (!decode(in, message) || !in.ok() || in.remaining() != 0) {
        return DecodeError::BadPayload;
    }
    return Packet{.header = header, .payload = std::move(message)};
}

} // namespace

PacketType payloadType(const Payload &payload)
{
    return std::visit(
        [](const auto &message) {
            return std::remove_cvref_t<decltype(message)>::Type;
        },
        payload);
}

bool encodePacket(const Packet &packet, ByteWriter &out)
{
    PacketHeader header = packet.header;
    header.type = payloadType(packet.payload);
    header.flags = isReliable(header.type) ? PacketFlag::Reliable : 0;
    header.payloadSize = 0;
    const std::size_t start = out.size();
    encodeHeader(header, out);
    std::visit([&out](const auto &message) { encode(message, out); },
               packet.payload);
    if (!out.ok()) {
        return false;
    }
    out.patchU16(start + PayloadSizeOffset,
                 static_cast<std::uint16_t>(out.size() - start - HeaderSize));
    return out.ok();
}

std::variant<Packet, DecodeError>
decodePacket(std::span<const std::uint8_t> datagram, Side sender)
{
    const auto decoded = decodeHeader(datagram);
    if (const auto *error = std::get_if<DecodeError>(&decoded)) {
        return *error;
    }
    const auto &header = std::get<PacketHeader>(decoded);
    if (!canSend(sender, header.type)) {
        return DecodeError::WrongSender;
    }
    ByteReader in(datagram.subspan(HeaderSize));
    switch (header.type) {
    case PacketType::Connect:
        return decodePayload<Connect>(header, in);
    case PacketType::Accept:
        return decodePayload<Accept>(header, in);
    case PacketType::Reject:
        return decodePayload<Reject>(header, in);
    case PacketType::Disconnect:
        return decodePayload<Disconnect>(header, in);
    case PacketType::Ack:
        return decodePayload<Ack>(header, in);
    case PacketType::Input:
        return decodePayload<Input>(header, in);
    case PacketType::Snapshot:
        return decodePayload<SnapshotPart>(header, in);
    case PacketType::Welcome:
        return decodePayload<Welcome>(header, in);
    case PacketType::GameEvent:
        return decodePayload<GameEvent>(header, in);
    }
    return DecodeError::UnknownType;
}

} // namespace rtype::protocol
