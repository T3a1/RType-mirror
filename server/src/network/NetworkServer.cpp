/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** NetworkServer
*/

#include "network/NetworkServer.hpp"
#include "common/protocol/PacketHeader.hpp"
#include "common/protocol/PacketType.hpp"
#include "common/protocol/ReliableChannel.hpp"
#include "engine/Logger.hpp"

namespace server {

void NetworkServer::start()
{
    receive();
    m_serverThread = std::thread([this] {
        try {
            m_io.run();
        } catch (const std::exception &e) {
            BB_ERROR(e.what());
        }
    });
}

void NetworkServer::receive()
{
    m_socket.async_receive_from(
        asio::buffer(m_buffer), m_remote,
        [this](const asio::error_code &error, std::size_t size) {
            onReceive(error, size);
        });
}

void NetworkServer::handleDatagram(std::size_t size)
{
    BB_DEBUG("Size " << size << " bytes received from " << m_remote.address()
                     << ":" << m_remote.port());
    const auto result = rtype::protocol::decodePacket(
        {m_buffer.data(), size}, rtype::protocol::Side::Client);
    const auto *packet = std::get_if<rtype::protocol::Packet>(&result);
    if (packet == nullptr) {
        const auto error = std::get<rtype::protocol::DecodeError>(result);
        BB_DEBUG("Received wrong datagram "
                 << rtype::protocol::toString(error));
        return;
    }
    auto it = m_sessions.find(m_remote);
    if (it == m_sessions.end()) {
        if (std::holds_alternative<rtype::protocol::Connect>(packet->payload)) {
            handleNewPeer();
        }
        return;
    }
    handleSessionPacket(it->second);
}

void NetworkServer::handleNewPeer()
{
    if (m_sessions.size() >= rtype::protocol::MaxPlayers) {
        BB_DEBUG("Too many clients");
        return;
    }
    m_sessions[m_remote] =
        Session{m_newCliendId++, rtype::protocol::ReliableChannel(),
                std::chrono::steady_clock::now()};
    BB_INFO("Registered new client " << m_sessions[m_remote].id);
}

void NetworkServer::handleSessionPacket(Session &session)
{
    BB_DEBUG("Received packet from known client " << session.id);
}

void NetworkServer::onReceive(const asio::error_code &error, std::size_t size)
{
    if (error == asio::error::operation_aborted) {
        return;
    }
    if (!error) {
        handleDatagram(size);
    }
    receive();
}

void NetworkServer::stop()
{
    m_io.stop();
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }
}

} // namespace server
