/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** IUdpSocket
*/

#ifndef ENGINE_NETWORK_IUDPSOCKET_HPP
#define ENGINE_NETWORK_IUDPSOCKET_HPP

#include "engine/network/Endpoint.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <system_error>

namespace engine::net {

/**
 * @class NetworkError
 * @brief Thrown when a socket cannot be opened or bound, or a host name
 * cannot be resolved. Sending and receiving never throw.
 **/
class NetworkError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

/**
 * @struct Datagram
 * @brief Where a received datagram comes from and how long it is.
 * @param sender The endpoint that sent it.
 * @param size Its size in bytes, at most the size of the receive buffer.
 **/
struct Datagram {
    Endpoint sender;
    std::size_t size = 0;
};

/**
 * @class IUdpSocket
 * @brief A bound UDP socket, independent of the network library.
 *
 * The server and the client only talk to this interface. Each
 * implementation lives in its own directory (Asio today, in
 * common/src/network/asio/, the only place that includes the library's
 * headers); CMake picks one with RTYPE_NETWORK_BACKEND, and
 * rtype::network::openUdpSocket() creates it.
 *
 * A socket is used by one thread at a time (the network thread). Nothing
 * runs in the background: datagrams are only read inside receive(), so the
 * caller keeps its own loop and timers.
 **/
class IUdpSocket {
  public:
    IUdpSocket() = default;
    virtual ~IUdpSocket() = default;

    IUdpSocket(const IUdpSocket &) = delete;
    IUdpSocket &operator=(const IUdpSocket &) = delete;
    IUdpSocket(IUdpSocket &&) = delete;
    IUdpSocket &operator=(IUdpSocket &&) = delete;

    /**
     * @brief Sends one datagram, without blocking.
     * @param to The recipient.
     * @param data The bytes to send.
     * @return An empty error code on success, the reason otherwise. The
     * datagram is dropped on failure, as UDP would.
     **/
    virtual std::error_code sendTo(const Endpoint &to,
                                   std::span<const std::uint8_t> data) = 0;

    /**
     * @brief Waits for one datagram, at most for a timeout. Receive errors
     * (for example an ICMP "port unreachable") are skipped.
     * @param buffer Where the datagram is written. A longer datagram is
     * truncated to the buffer size.
     * @param timeout How long to wait; zero only polls, without blocking.
     * @return The sender and the size, or nothing if no datagram arrived
     * before the timeout.
     **/
    virtual std::optional<Datagram>
    receive(std::span<std::uint8_t> buffer,
            std::chrono::milliseconds timeout) = 0;

    /**
     * @brief Returns the local endpoint the socket is bound to.
     * @return The local address and port (the port chosen by the system if
     * the socket was opened on port 0).
     **/
    [[nodiscard]] virtual Endpoint localEndpoint() const = 0;
};

} // namespace engine::net

#endif // ENGINE_NETWORK_IUDPSOCKET_HPP
