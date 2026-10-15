/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Connection
*/

#ifndef COMMON_PROTOCOL_CONNECTION_HPP
#define COMMON_PROTOCOL_CONNECTION_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace rtype::protocol {

inline constexpr std::uint16_t DefaultPort = 4242;

inline constexpr std::chrono::milliseconds ConnectRetryInterval{500};
inline constexpr std::chrono::milliseconds ResendInterval{100};
inline constexpr std::chrono::milliseconds PeerTimeout{5000};
inline constexpr std::size_t MaxPendingReliable = 256;

inline constexpr std::uint16_t FirstReliableSequence = 1;

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_CONNECTION_HPP
