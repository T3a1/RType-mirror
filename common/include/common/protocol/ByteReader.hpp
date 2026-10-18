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

/**
 * @class ByteReader
 * @brief Reads little-endian numbers from untrusted bytes.
 *
 * Reading past the end returns 0 and sets ok() to false for good, so a
 * decoder reads every field and checks ok() once at the end.
 **/
class ByteReader {
  public:
    /**
     * @brief Creates a reader at the start of a byte buffer.
     * @param bytes The bytes to read; they must outlive the reader.
     **/
    explicit ByteReader(std::span<const std::uint8_t> bytes);

    /**
     * @brief Reads one byte.
     * @return The byte, or 0 past the end.
     **/
    [[nodiscard]] std::uint8_t u8();

    /**
     * @brief Reads a 16-bit unsigned integer, low byte first.
     * @return The value (2 bytes), or 0 past the end.
     **/
    [[nodiscard]] std::uint16_t u16();

    /**
     * @brief Reads a 32-bit unsigned integer, low byte first.
     * @return The value (4 bytes), or 0 past the end.
     **/
    [[nodiscard]] std::uint32_t u32();

    /**
     * @brief Reads a float stored as its IEEE 754 binary32 bits.
     * @return The value (4 bytes), or 0 past the end. It may be NaN or
     * infinite: the caller validates it.
     **/
    [[nodiscard]] float f32();

    /**
     * @brief Tells whether every read so far stayed inside the buffer.
     * @return `true` if no read went past the end.
     **/
    [[nodiscard]] bool ok() const;

    /**
     * @brief Returns the number of bytes not read yet.
     * @return The remaining size, in bytes.
     **/
    [[nodiscard]] std::size_t remaining() const;

  private:
    std::uint32_t readLittleEndian(std::size_t byteCount);

    std::span<const std::uint8_t> m_bytes;
    std::size_t m_position = 0;
    bool m_ok = true;
};

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_BYTEREADER_HPP
