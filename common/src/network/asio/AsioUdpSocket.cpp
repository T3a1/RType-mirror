/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** AsioUdpSocket
*/

#include "AsioUdpSocket.hpp"
#include "common/network/UdpSocket.hpp"
#include "engine/Logger.hpp"

#include <string>

namespace rtype::network {

AsioUdpSocket::AsioUdpSocket(std::uint16_t port)
{
    asio::error_code error;
    if (m_socket.open(asio::ip::udp::v4(), error)) {
        throw engine::net::NetworkError("Cannot open a UDP socket: " +
                                        error.message());
    }
    if (m_socket.bind({asio::ip::udp::v4(), port}, error)) {
        throw engine::net::NetworkError("Cannot bind UDP port " +
                                        std::to_string(port) + ": " +
                                        error.message());
    }
    if (m_socket.non_blocking(true, error)) {
        throw engine::net::NetworkError(
            "Cannot make the UDP socket non-blocking: " + error.message());
    }
}

std::error_code AsioUdpSocket::sendTo(const engine::net::Endpoint &to,
                                      std::span<const std::uint8_t> data)
{
    asio::error_code error;
    m_socket.send_to(asio::buffer(data.data(), data.size()), fromEndpoint(to),
                     0, error);
    return error;
}

std::optional<engine::net::Datagram>
AsioUdpSocket::receive(std::span<std::uint8_t> buffer,
                       std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (true) {
        asio::ip::udp::endpoint sender;
        asio::error_code error;
        const std::size_t size = m_socket.receive_from(
            asio::buffer(buffer.data(), buffer.size()), sender, 0, error);
        if (!error) {
            return engine::net::Datagram{.sender = toEndpoint(sender),
                                         .size = size};
        }
        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            return std::nullopt;
        }
        if (error == asio::error::would_block &&
            !waitReadable(deadline - now)) {
            return std::nullopt;
        }
    }
}

engine::net::Endpoint AsioUdpSocket::localEndpoint() const
{
    asio::error_code error;
    return toEndpoint(m_socket.local_endpoint(error));
}

bool AsioUdpSocket::waitReadable(std::chrono::steady_clock::duration timeout)
{
    bool ready = false;
    m_socket.async_wait(
        asio::socket_base::wait_read,
        [&ready](const asio::error_code &error) { ready = !error; });
    m_io.restart();
    m_io.run_for(timeout);
    if (!m_io.stopped()) {
        asio::error_code error;
        if (m_socket.cancel(error)) {
            BB_DEBUG("Cannot cancel the wait: " << error.message());
        }
        m_io.run();
    }
    return ready;
}

engine::net::Endpoint
AsioUdpSocket::toEndpoint(const asio::ip::udp::endpoint &e)
{
    return {.address = e.address().to_v4().to_uint(), .port = e.port()};
}

asio::ip::udp::endpoint
AsioUdpSocket::fromEndpoint(const engine::net::Endpoint &e)
{
    return {asio::ip::address_v4(e.address), e.port};
}

std::unique_ptr<engine::net::IUdpSocket> openUdpSocket(std::uint16_t port)
{
    return std::make_unique<AsioUdpSocket>(port);
}

engine::net::Endpoint resolve(const std::string &host, std::uint16_t port)
{
    asio::io_context io;
    asio::ip::udp::resolver resolver(io);
    asio::error_code error;
    const auto results = resolver.resolve(asio::ip::udp::v4(), host,
                                          std::to_string(port), error);
    if (error || results.empty()) {
        throw engine::net::NetworkError(
            "Cannot resolve " + host + ": " +
            (error ? error.message() : std::string("no IPv4 address")));
    }
    return AsioUdpSocket::toEndpoint(results.begin()->endpoint());
}

} // namespace rtype::network
