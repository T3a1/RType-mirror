/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** NetworkServer
*/

#ifndef SERVER_NETWORKSERVER_HPP
#define SERVER_NETWORKSERVER_HPP
#include "common/protocol/ByteWriter.hpp"
#include "common/protocol/Connection.hpp"
#include "common/protocol/Packet.hpp"
#include "common/protocol/ReliableChannel.hpp"
#include "engine/MessageQueue.hpp"
#include "network/NetworkMessages.hpp"
#include <array>
#include <asio.hpp>
#include <cstdint>
#include <map>
#include <thread>

namespace server {

struct Session {
    std::uint32_t id{};
    rtype::protocol::ReliableChannel channel;
    rtype::protocol::ReliableChannel::TimePoint lastHeard{};
};

class NetworkServer {
  public:
    NetworkServer(engine::MessageQueue<InboundMessage> &inbound,
                  engine::MessageQueue<OutboundMessage> &outbound,
                  std::uint16_t port = rtype::protocol::DefaultPort)
        : m_io(),
          m_socket(m_io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
          m_inbound(inbound), m_outbound(outbound) {};
    ~NetworkServer() { stop(); };

    void start();
    void stop();

  private:
    asio::io_context m_io;
    asio::ip::udp::socket m_socket;
    engine::MessageQueue<InboundMessage> &m_inbound;
    engine::MessageQueue<OutboundMessage> &m_outbound;
    std::thread m_serverThread;
    std::array<std::uint8_t, rtype::protocol::MaxPacketSize> m_buffer{};
    asio::ip::udp::endpoint m_remote;
    std::map<asio::ip::udp::endpoint, Session> m_sessions{};
    std::uint32_t m_newCliendId = 1;

    void receive();
    void onReceive(const asio::error_code &error, std::size_t size);
    void handleDatagram(std::size_t size);
    void handleNewPeer();
    void handleSessionPacket(Session &session);
};
} // namespace server

#endif // SERVER_NETWORKSERVER_HPP
