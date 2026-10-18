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

inline constexpr std::size_t MaxPacketSize = 1200;

/**
 * @class ByteWriter
 * @brief Serializes numbers in little-endian order into a fixed buffer of
 * MaxPacketSize bytes.
 *
 * A write that does not fit sets ok() to false for good and writes nothing
 * more, so an encoder writes every field and checks ok() once at the end.
 **/
class ByteWriter {
  public:
    /**
     * @brief Appends one byte.
     * @param value The byte to write.
     **/
    void u8(std::uint8_t value);

    /**
     * @brief Appends a 16-bit unsigned integer, low byte first.
     * @param value The value to write (2 bytes).
     **/
    void u16(std::uint16_t value);

    /**
     * @brief Appends a 32-bit unsigned integer, low byte first.
     * @param value The value to write (4 bytes).
     **/
    void u32(std::uint32_t value);

    /**
     * @brief Appends a float as its IEEE 754 binary32 bits, low byte first.
     * @param value The value to write (4 bytes).
     **/
    void f32(float value);

    /**
     * @brief Overwrites 2 bytes already written, for a size only known once
     * the rest has been written.
     * @param offset The position of the first byte to overwrite, in bytes.
     * @param value The value to write, low byte first.
     **/
    void patchU16(std::size_t offset, std::uint16_t value);

    /**
     * @brief Tells whether every write so far has fit in the buffer.
     * @return `true` if no write has failed.
     **/
    [[nodiscard]] bool ok() const;

    /**
     * @brief Returns the number of bytes written.
     * @return The size of the written data, in bytes.
     **/
    [[nodiscard]] std::size_t size() const;

    /**
     * @brief Returns the bytes written so far, ready to be sent.
     * @return A view on the first size() bytes of the buffer.
     **/
    [[nodiscard]] std::span<const std::uint8_t> data() const;

  private:
    void writeLittleEndian(std::uint32_t value, std::size_t byteCount);

    std::array<std::uint8_t, MaxPacketSize> m_bytes{};
    std::size_t m_size = 0;
    bool m_ok = true;
};

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_BYTEWRITER_HPP
