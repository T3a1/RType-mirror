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
#include <string>

namespace engine::net {

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
        std::string text;
        for (int shift = 24; shift >= 0; shift -= 8) {
            text += std::to_string((address >> shift) & 0xFFU);
            text += shift == 0 ? ':' : '.';
        }
        return text + std::to_string(port);
    }
};

} // namespace engine::net

#endif // ENGINE_NETWORK_ENDPOINT_HPP
