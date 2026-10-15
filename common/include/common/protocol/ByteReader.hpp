/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** ByteReader
*/

#ifndef COMMON_PROTOCOL_BYTEREADER_HPP
#define COMMON_PROTOCOL_BYTEREADER_HPP

#include <cstddef>
#include <cstdint>
#include <span>

namespace rtype::protocol {

// Reads little-endian numbers from untrusted bytes. Reading past the end
// returns 0 and sets ok() to false for good, so a decoder reads every
// field and checks ok() once at the end.
class ByteReader {
  public:
    explicit ByteReader(std::span<const std::uint8_t> bytes);

    [[nodiscard]] std::uint8_t u8();
    [[nodiscard]] std::uint16_t u16();
    [[nodiscard]] std::uint32_t u32();
    [[nodiscard]] float f32();

    [[nodiscard]] bool ok() const;
    [[nodiscard]] std::size_t remaining() const;

  private:
    std::uint32_t readLittleEndian(std::size_t byteCount);

    std::span<const std::uint8_t> m_bytes;
    std::size_t m_position = 0;
    bool m_ok = true;
};

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_BYTEREADER_HPP
