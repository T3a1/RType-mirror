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

using TimePoint = rtype::protocol::ReliableChannel::TimePoint;

/**
 * @struct Session
 * @brief The network thread's state for one connected client.
 * @param id The ClientId shared with the game thread.
 * @param channel The reliability state of the connection.
 * @param lastHeard When the last valid packet was received, for the timeout.
 * @param kicked Set when the client must be disconnected at the next
 * update.
 **/
struct Session {
    std::uint32_t id{};
    rtype::protocol::ReliableChannel channel;
    TimePoint lastHeard;
    bool kicked = false;
};

using SessionIterator = std::map<asio::ip::udp::endpoint, Session>::iterator;

/**
 * @class NetworkServer
 * @brief The server's network thread: the UDP socket, the sessions, the
 * reliability and the timeouts.
 *
 * It talks to the game thread only through the two queues. Everything else
 * is touched by the network thread alone, so it needs no lock. Call start()
 * once, and stop() (or let the destructor do it) at shutdown.
 **/
class NetworkServer {
  public:
    /**
     * @brief Opens the UDP socket on a port. Nothing runs before start().
     * @param inbound The queue to the game thread.
     * @param outbound The queue from the game thread.
     * @param port The UDP port to listen on.
     * @throws asio::system_error If the port cannot be bound.
     **/
    NetworkServer(engine::MessageQueue<InboundMessage> &inbound,
                  engine::MessageQueue<OutboundMessage> &outbound,
                  std::uint16_t port = rtype::protocol::DefaultPort)
        : m_socket(m_io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
          m_inbound(inbound), m_outbound(outbound)
    {
    }
    NetworkServer(const NetworkServer &) = delete;
    NetworkServer &operator=(const NetworkServer &) = delete;
    NetworkServer(NetworkServer &&) = delete;
    NetworkServer &operator=(NetworkServer &&) = delete;
    /**
     * @brief Stops the network thread if it still runs.
     **/
    ~NetworkServer() { stop(); }

    /**
     * @brief Starts the network thread: receiving datagrams and the update
     * every 10 ms (resends, timeouts, outbound queue).
     **/
    void start();

    /**
     * @brief Sends Disconnect{ServerClosing} to every client from the
     * network thread, stops it and waits for it. Safe to call twice.
     **/
    void stop();

  private:
    asio::io_context m_io;
    asio::ip::udp::socket m_socket;
    engine::MessageQueue<InboundMessage> &m_inbound;
    engine::MessageQueue<OutboundMessage> &m_outbound;
    std::thread m_serverThread;
    std::array<std::uint8_t, rtype::protocol::MaxPacketSize> m_buffer{};
    asio::ip::udp::endpoint m_remote;
    std::map<asio::ip::udp::endpoint, Session> m_sessions;
    std::uint32_t m_newCliendId = 1;
    asio::steady_timer m_timer{m_io};

    void receive();
    void onReceive(const asio::error_code &error, std::size_t size);
    void handleDatagram(std::size_t size);
    void handleNewPeer(TimePoint now);
    void handleSessionPacket(SessionIterator it,
                             const rtype::protocol::Packet &packet,
                             TimePoint now);
    void send(SessionIterator it, const rtype::protocol::Payload &payload,
              TimePoint now);
    void sendPacket(const asio::ip::udp::endpoint &endpoint,
                    const rtype::protocol::Packet &packet);
    void pushInbound(InboundMessage message);
    void removeSession(SessionIterator it,
                       rtype::protocol::DisconnectReason reason);
    void scheduleUpdate();
    void update(TimePoint now);
    static bool isRecipient(const OutboundMessage &message,
                            std::uint32_t clientId);
};
} // namespace server

#endif // SERVER_NETWORKSERVER_HPP
