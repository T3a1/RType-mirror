/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** UdpSocket
*/

#ifndef COMMON_NETWORK_UDPSOCKET_HPP
#define COMMON_NETWORK_UDPSOCKET_HPP

#include "engine/network/Endpoint.hpp"
#include "engine/network/IUdpSocket.hpp"

#include <cstdint>
#include <memory>
#include <string>

/**
 * @namespace rtype::network
 * @brief Entry points of the network backend chosen with
 * RTYPE_NETWORK_BACKEND. Each backend defines these functions in
 * common/src/network/<backend>/.
 **/
namespace rtype::network {

/**
 * @brief Opens a non-blocking IPv4 UDP socket bound to a port on every
 * interface.
 * @param port The local port; 0 lets the system pick one (clients).
 * @return The socket.
 * @throws engine::net::NetworkError If the socket cannot be opened or the
 * port cannot be bound.
 **/
std::unique_ptr<engine::net::IUdpSocket> openUdpSocket(std::uint16_t port = 0);

/**
 * @brief Resolves a host name or an IPv4 address to an endpoint.
 * @param host The host name ("localhost") or address ("127.0.0.1").
 * @param port The UDP port.
 * @return The first IPv4 endpoint found.
 * @throws engine::net::NetworkError If the host cannot be resolved.
 **/
engine::net::Endpoint resolve(const std::string &host, std::uint16_t port);

} // namespace rtype::network

#endif // COMMON_NETWORK_UDPSOCKET_HPP
