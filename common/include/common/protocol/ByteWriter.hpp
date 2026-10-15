/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** ByteWriter
*/

#ifndef COMMON_PROTOCOL_BYTEWRITER_HPP
#define COMMON_PROTOCOL_BYTEWRITER_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace rtype::protocol {

// Largest datagram either side sends or accepts, header included. Below
// the usual 1280-byte IPv6 minimum MTU, so packets are never fragmented.
inline constexpr std::size_t MaxPacketSize = 1200;

// Appends numbers to a fixed buffer in little-endian order. A write that
// does not fit sets ok() to false for good, and nothing more is written.
class ByteWriter {
  public:
    void u8(std::uint8_t value);
    void u16(std::uint16_t value);
    void u32(std::uint32_t value);
    void f32(float value);
    void patchU16(std::size_t offset, std::uint16_t value);

    [[nodiscard]] bool ok() const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] std::span<const std::uint8_t> data() const;

  private:
    void writeLittleEndian(std::uint32_t value, std::size_t byteCount);

    std::array<std::uint8_t, MaxPacketSize> m_bytes{};
    std::size_t m_size = 0;
    bool m_ok = true;
};

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_BYTEWRITER_HPP
