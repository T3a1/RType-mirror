/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Endpoint
*/

#ifndef ENGINE_NETWORK_ENDPOINT_HPP
#define ENGINE_NETWORK_ENDPOINT_HPP

#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace engine::net {

/**
 * @brief Formats an IPv4 address.
 * @param address The address in host byte order.
 * @return The address as "a.b.c.d".
 **/
[[nodiscard]] inline std::string addressToString(std::uint32_t address)
{
    std::string text;
    for (unsigned part = 4; part > 0; --part) {
        text += std::to_string((address >> ((part - 1) * 8U)) & 0xFFU);
        if (part != 1) {
            text += '.';
        }
    }
    return text;
}

/**
 * @brief Reads an IPv4 address.
 * @param text The address as "a.b.c.d".
 * @return The address in host byte order, or nothing if it is not valid.
 **/
[[nodiscard]] inline std::optional<std::uint32_t>
parseAddress(std::string_view text)
{
    constexpr int Parts = 4;
    constexpr std::size_t MaxDigits = 3;
    constexpr unsigned MaxPart = 255;
    constexpr unsigned PartBits = 8;
    std::uint32_t address = 0;
    std::string_view rest = text;
    for (int part = 0; part < Parts; ++part) {
        const std::size_t dot = rest.find('.');
        const bool last = part + 1 == Parts;
        if (last == (dot != std::string_view::npos)) {
            return std::nullopt;
        }
        const std::string_view digits = rest.substr(0, dot);
        if (digits.empty() || digits.size() > MaxDigits) {
            return std::nullopt;
        }
        unsigned value = 0;
        for (const char digit : digits) {
            if (digit < '0' || digit > '9') {
                return std::nullopt;
            }
            value = (value * 10U) + static_cast<unsigned>(digit - '0');
        }
        if (value > MaxPart) {
            return std::nullopt;
        }
        address = (address << PartBits) | value;
        rest = last ? std::string_view{} : rest.substr(dot + 1);
    }
    return address;
}

/**
 * @struct Endpoint
 * @brief An IPv4 address and a UDP port, independent of the network library.
 *
 * Endpoints are totally ordered, so they can key a std::map (the server's
 * session table).
 * @param address The IPv4 address in host byte order (127.0.0.1 is
 * 0x7F000001).
 * @param port The UDP port in host byte order.
 **/
struct Endpoint {
    std::uint32_t address = 0;
    std::uint16_t port = 0;

    auto operator<=>(const Endpoint &) const = default;

    /**
     * @brief Formats the endpoint for logs.
     * @return The endpoint as "a.b.c.d:port".
     **/
    [[nodiscard]] std::string toString() const
    {
        return addressToString(address) + ':' + std::to_string(port);
    }
};

} // namespace engine::net

#endif // ENGINE_NETWORK_ENDPOINT_HPP
