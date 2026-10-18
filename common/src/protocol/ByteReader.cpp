/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** ByteReader
*/

#include "common/protocol/ByteReader.hpp"

#include <bit>
#include <limits>

namespace rtype::protocol {

static_assert(std::numeric_limits<float>::is_iec559,
              "the protocol sends floats as IEEE 754 binary32");

ByteReader::ByteReader(std::span<const std::uint8_t> bytes) : m_bytes(bytes) {}

std::uint8_t ByteReader::u8()
{
    return static_cast<std::uint8_t>(readLittleEndian(1));
}

std::uint16_t ByteReader::u16()
{
    return static_cast<std::uint16_t>(readLittleEndian(2));
}

std::uint32_t ByteReader::u32() { return readLittleEndian(4); }

float ByteReader::f32() { return std::bit_cast<float>(readLittleEndian(4)); }

bool ByteReader::ok() const { return m_ok; }

std::size_t ByteReader::remaining() const
{
    return m_bytes.size() - m_position;
}

std::uint32_t ByteReader::readLittleEndian(std::size_t byteCount)
{
    if (!m_ok || remaining() < byteCount) {
        m_ok = false;
        return 0;
    }
    std::uint32_t value = 0;
    for (std::size_t i = 0; i < byteCount; ++i) {
        value |= static_cast<std::uint32_t>(m_bytes[m_position + i]) << (8 * i);
    }
    m_position += byteCount;
    return value;
}

} // namespace rtype::protocol
