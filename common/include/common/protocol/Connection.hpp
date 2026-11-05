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

/** @brief The UDP port of the server unless another one is chosen. */
inline constexpr std::uint16_t DefaultPort = 4242;

/** @brief How often a client repeats Connect until it gets an answer. */
inline constexpr std::chrono::milliseconds ConnectRetryInterval{500};
/** @brief How long a reliable packet waits for its ack before being resent. */
inline constexpr std::chrono::milliseconds ResendInterval{100};
/** @brief A peer that sends nothing for this long is considered gone. */
inline constexpr std::chrono::milliseconds PeerTimeout{5000};
/** @brief How many reliable packets may wait for an ack, for each peer. */
inline constexpr std::size_t MaxPendingReliable = 256;

/** @brief The sequence of the first reliable packet sent to a peer. */
inline constexpr std::uint16_t FirstReliableSequence = 1;

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_CONNECTION_HPP
