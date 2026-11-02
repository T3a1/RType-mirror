/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** AsioUdpSocket
*/

#ifndef COMMON_NETWORK_ASIO_ASIOUDPSOCKET_HPP
#define COMMON_NETWORK_ASIO_ASIOUDPSOCKET_HPP

#include "engine/network/IUdpSocket.hpp"

#include <asio.hpp>

#include <cstdint>

namespace rtype::network {

/**
 * @class AsioUdpSocket
 * @brief IUdpSocket implemented with standalone Asio. Private to
 * r-type_common: nothing outside common/src/network/asio/ sees Asio.
 *
 * The socket is non-blocking. receive() waits with an async_wait on the
 * socket and io_context::run_for(), so it never blocks past its timeout.
 **/
class AsioUdpSocket final : public engine::net::IUdpSocket {
  public:
    /**
     * @brief Opens the socket and binds it.
     * @param port The local port, 0 for any.
     * @throws engine::net::NetworkError If opening or binding fails.
     **/
    explicit AsioUdpSocket(std::uint16_t port);

    std::error_code sendTo(const engine::net::Endpoint &to,
                           std::span<const std::uint8_t> data) override;
    std::optional<engine::net::Datagram>
    receive(std::span<std::uint8_t> buffer,
            std::chrono::milliseconds timeout) override;
    [[nodiscard]] engine::net::Endpoint localEndpoint() const override;

    static engine::net::Endpoint toEndpoint(const asio::ip::udp::endpoint &e);
    static asio::ip::udp::endpoint fromEndpoint(const engine::net::Endpoint &e);

  private:
    bool waitReadable(std::chrono::steady_clock::duration timeout);

    asio::io_context m_io;
    asio::ip::udp::socket m_socket{m_io};
};

} // namespace rtype::network

#endif // COMMON_NETWORK_ASIO_ASIOUDPSOCKET_HPP
