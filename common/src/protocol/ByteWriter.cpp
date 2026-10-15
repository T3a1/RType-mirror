/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** ByteWriter
*/

#include "common/protocol/ByteWriter.hpp"

#include <bit>
#include <limits>

namespace rtype::protocol {

static_assert(std::numeric_limits<float>::is_iec559,
              "the protocol sends floats as IEEE 754 binary32");

void ByteWriter::u8(std::uint8_t value) { writeLittleEndian(value, 1); }

void ByteWriter::u16(std::uint16_t value) { writeLittleEndian(value, 2); }

void ByteWriter::u32(std::uint32_t value) { writeLittleEndian(value, 4); }

void ByteWriter::f32(float value)
{
    writeLittleEndian(std::bit_cast<std::uint32_t>(value), 4);
}

void ByteWriter::patchU16(std::size_t offset, std::uint16_t value)
{
    if (offset > m_size || m_size - offset < 2) {
        m_ok = false;
        return;
    }
    m_bytes[offset] = static_cast<std::uint8_t>(value);
    m_bytes[offset + 1] = static_cast<std::uint8_t>(value >> 8U);
}

bool ByteWriter::ok() const { return m_ok; }

std::size_t ByteWriter::size() const { return m_size; }

std::span<const std::uint8_t> ByteWriter::data() const
{
    return std::span(m_bytes).first(m_size);
}

void ByteWriter::writeLittleEndian(std::uint32_t value, std::size_t byteCount)
{
    if (!m_ok || MaxPacketSize - m_size < byteCount) {
        m_ok = false;
        return;
    }
    for (std::size_t i = 0; i < byteCount; ++i) {
        m_bytes[m_size + i] = static_cast<std::uint8_t>(value >> (8U * i));
    }
    m_size += byteCount;
}

} // namespace rtype::protocol
