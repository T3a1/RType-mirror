/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PacketHeader
*/

#include "common/protocol/PacketHeader.hpp"
#include "common/protocol/ByteReader.hpp"

namespace rtype::protocol {

void encodeHeader(const PacketHeader &header, ByteWriter &out)
{
    out.u8(Magic[0]);
    out.u8(Magic[1]);
    out.u8(ProtocolVersion);
    out.u8(static_cast<std::uint8_t>(header.type));
    out.u8(header.flags);
    out.u8(0); // reserved
    out.u16(header.sequence);
    out.u16(header.ack);
    out.u16(header.payloadSize);
}

std::variant<PacketHeader, DecodeError>
decodeHeader(std::span<const std::uint8_t> datagram)
{
    if (datagram.size() > MaxPacketSize) {
        return DecodeError::TooLarge;
    }
    if (datagram.size() < HeaderSize) {
        return DecodeError::TooShort;
    }
    ByteReader in(datagram.first(HeaderSize));
    const std::uint8_t magic0 = in.u8();
    const std::uint8_t magic1 = in.u8();
    if (magic0 != Magic[0] || magic1 != Magic[1]) {
        return DecodeError::BadMagic;
    }
    if (in.u8() != ProtocolVersion) {
        return DecodeError::BadVersion;
    }
    const std::uint8_t type = in.u8();
    if (!isKnownPacketType(type)) {
        return DecodeError::UnknownType;
    }
    PacketHeader header;
    header.type = static_cast<PacketType>(type);
    header.flags = in.u8();
    const bool reliable = (header.flags & PacketFlag::Reliable) != 0;
    if ((header.flags & PacketFlag::All) != header.flags ||
        reliable != isReliable(header.type)) {
        return DecodeError::BadFlags;
    }
    if (in.u8() != 0) {
        return DecodeError::BadReserved;
    }
    header.sequence = in.u16();
    header.ack = in.u16();
    header.payloadSize = in.u16();
    if (header.payloadSize != datagram.size() - HeaderSize) {
        return DecodeError::BadPayloadSize;
    }
    return header;
}

} // namespace rtype::protocol
